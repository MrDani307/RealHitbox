#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include <algorithm>
#include <vector>

using namespace geode::prelude;

/*
    Real Hitbox 2.1

    Marker:
        m_hasExtendedCollision (property 511)

    The marker is stored on the actual selected object.
    We do NOT permanently convert the object's type in the editor.

    During gameplay:
        decorative/non-solid marked objects are temporarily presented
        to the vanilla collision routine as solid blocks.
*/

// ============================================================
// Runtime collision hook
// ============================================================

class $modify(RealHitboxCollision, GJBaseGameLayer) {
    struct Fields {
        std::vector<GameObject*> realHitboxes;
    };

    void addObject(GameObject* object) {
        GJBaseGameLayer::addObject(object);

        if (!object)
            return;

        // m_hasExtendedCollision is used as the persistent
        // Real Hitbox marker.
        if (!object->m_hasExtendedCollision)
            return;

        // Already a real solid? Vanilla collision already handles it.
        if (object->m_objectType == GameObjectType::Solid)
            return;

        if (std::find(
                m_fields->realHitboxes.begin(),
                m_fields->realHitboxes.end(),
                object
            ) == m_fields->realHitboxes.end()) {

            m_fields->realHitboxes.push_back(object);
        }
    }

    void collisionCheckObjects(
        PlayerObject* player,
        gd::vector<GameObject*>* objects,
        int objectCount,
        float dt
    ) {
        if (!player || !objects || m_fields->realHitboxes.empty()) {
            GJBaseGameLayer::collisionCheckObjects(
                player,
                objects,
                objectCount,
                dt
            );
            return;
        }

        if (!m_objects) {
            GJBaseGameLayer::collisionCheckObjects(
                player,
                objects,
                objectCount,
                dt
            );
            return;
        }

        const size_t originalSize = objects->size();

        struct SavedState {
            GameObject* object;

            GameObjectType objectType;

            bool isNoTouch;
            bool isPassable;
            bool isDecoration;
            bool isDecoration2;
        };

        std::vector<SavedState> changed;
        changed.reserve(m_fields->realHitboxes.size());

        for (auto hitbox : m_fields->realHitboxes) {
            if (!hitbox)
                continue;

            /*
                Make sure the pointer still belongs to this game's
                actual object array before touching it.

                This prevents stale pointers if an object was removed.
            */
            bool stillExists = false;

            for (auto current : CCArrayExt<GameObject*>(m_objects)) {
                if (current == hitbox) {
                    stillExists = true;
                    break;
                }
            }

            if (!stillExists)
                continue;

            /*
                If it somehow became a real solid already, there is
                no reason to inject it again.
            */
            if (hitbox->m_objectType == GameObjectType::Solid)
                continue;

            /*
                Do not use GameObject::setType() here.

                We only change the raw collision state for the duration
                of the vanilla collision call. The editor object itself
                remains a decoration outside this function.
            */
            changed.push_back({
                hitbox,
                hitbox->m_objectType,
                hitbox->m_isNoTouch,
                hitbox->m_isPassable,
                hitbox->m_isDecoration,
                hitbox->m_isDecoration2
            });

            // Present it to GD's collision code as a normal solid block.
            hitbox->m_objectType = GameObjectType::Solid;
            hitbox->m_isNoTouch = false;
            hitbox->m_isPassable = false;
            hitbox->m_isDecoration = false;
            hitbox->m_isDecoration2 = false;

            // Force the collision rectangles to be recalculated.
            hitbox->setObjectRectDirty(true);
            hitbox->setOrientedRectDirty(true);

            if (std::find(
                    objects->begin(),
                    objects->end(),
                    hitbox
                ) == objects->end()) {

                objects->push_back(hitbox);
            }
        }

        /*
            objectCount is part of the actual function signature,
            so it must match the injected vector size.
        */
        const int newObjectCount =
            static_cast<int>(objects->size());

        // Vanilla Geometry Dash collision processing.
        GJBaseGameLayer::collisionCheckObjects(
            player,
            objects,
            newObjectCount,
            dt
        );

        /*
            Restore every modified object immediately.

            This is the important part preventing the editor crashes
            caused by leaving a decoration permanently converted.
        */
        for (auto const& state : changed) {
            if (!state.object)
                continue;

            state.object->m_objectType = state.objectType;
            state.object->m_isNoTouch = state.isNoTouch;
            state.object->m_isPassable = state.isPassable;
            state.object->m_isDecoration = state.isDecoration;
            state.object->m_isDecoration2 = state.isDecoration2;

            state.object->setObjectRectDirty(true);
            state.object->setOrientedRectDirty(true);
        }

        // Remove only the objects that this function injected.
        if (objects->size() > originalSize) {
            objects->resize(originalSize);
        }
    }
};


// ============================================================
// Editor
// ============================================================

class $modify(RealHitboxEditor, LevelEditorLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* hitboxButton = nullptr;
    };

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI))
            return false;

        auto menu = CCMenu::create();

        menu->setID("real-hitbox-menu");
        menu->setPosition({0.f, 0.f});

        // -----------------------------
        // Button
        // -----------------------------

        auto sprite =
            CCSprite::createWithSpriteFrameName(
                "GJ_plusBtn_001.png"
            );

        if (!sprite)
            sprite = CCSprite::create();

        sprite->setScale(0.55f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(
                RealHitboxEditor::onRealHitbox
            )
        );

        button->setID("real-hitbox-button");
        button->setPosition({28.f, 150.f});

        menu->addChild(button);

        // -----------------------------
        // Version
        // -----------------------------

        auto version =
            CCLabelBMFont::create(
                "2.1",
                "goldFont.fnt"
            );

        if (version) {
            version->setScale(0.42f);
            version->setPosition({28.f, 126.f});
            menu->addChild(version);
        }

        this->addChild(menu, 1000);

        m_fields->hitboxButton = button;

        return true;
    }

    void onRealHitbox(CCObject*) {
        if (!m_editorUI || !m_editorUI->m_selectedObjects) {
            return;
        }

        auto selected = m_editorUI->m_selectedObjects;

        if (selected->count() == 0) {
            FLAlertLayer::create(
                "Real Hitbox 2.1",
                "Select one or more objects first.",
                "OK"
            )->show();

            return;
        }

        bool allEnabled = true;
        int validCount = 0;

        // First determine current state.
        for (auto object : CCArrayExt<GameObject*>(selected)) {
            if (!object)
                continue;

            validCount++;

            if (!object->m_hasExtendedCollision) {
                allEnabled = false;
            }
        }

        if (validCount == 0) {
            return;
        }

        /*
            Toggle behavior:

            OFF -> ON:
                selected objects receive Real Hitbox

            ON -> OFF:
                selected objects lose Real Hitbox
        */
        const bool enable = !allEnabled;

        for (auto object : CCArrayExt<GameObject*>(selected)) {
            if (!object)
                continue;

            object->m_hasExtendedCollision = enable;

            /*
                Do not change:
                    m_objectType
                    m_isDecoration
                    m_isDecoration2
                    m_isPassable
                    m_isNoTouch

                Those are deliberately left intact in the editor.
            */
            object->setObjectRectDirty(true);
            object->setOrientedRectDirty(true);
        }

        /*
            Inform the editor by refreshing the selected objects.
            This does not change their visual appearance.
        */
        for (auto object : CCArrayExt<GameObject*>(selected)) {
            if (!object)
                continue;

            object->updateObjectEditorColor();
        }
    }
};
