#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

class $modify(RealHitboxEditor, LevelEditorLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* hitboxButton = nullptr;
    };

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI))
            return false;

        if (noUI)
            return true;

        auto menu = CCMenu::create();
        if (!menu)
            return true;

        menu->setID("real-hitbox-menu");
        menu->setPosition({0.f, 0.f});

        auto sprite = CCSprite::createWithSpriteFrameName(
            "GJ_plusBtn_001.png"
        );

        if (!sprite)
            sprite = CCSprite::create();

        if (!sprite)
            return true;

        sprite->setScale(0.55f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(RealHitboxEditor::onCreateBlockHitbox)
        );

        if (!button)
            return true;

        button->setID("create-block-hitbox");
        button->setPosition({28.f, 150.f});

        menu->addChild(button);

        // Version text
        auto version = CCLabelBMFont::create(
            "Real Hitbox 2.1",
            "goldFont.fnt"
        );

        if (version) {
            version->setScale(0.32f);
            version->setPosition({28.f, 125.f});
            menu->addChild(version);
        }

        this->addChild(menu, 1000);

        m_fields->hitboxButton = button;

        return true;
    }

    void onCreateBlockHitbox(CCObject*) {
        if (!m_editorUI || !m_editorUI->m_selectedObjects)
            return;

        auto selected = m_editorUI->m_selectedObjects;

        if (selected->count() == 0) {
            FLAlertLayer::create(
                "Real Hitbox 2.1",
                "Select one or more objects first.",
                "OK"
            )->show();

            return;
        }

        int changed = 0;

        for (auto object : CCArrayExt<GameObject*>(selected)) {
            if (!object)
                continue;

            /*
             * Turn the selected object itself into a solid object.
             *
             * No new GameObject is created here.
             */
            object->setType(GameObjectType::Solid);

            /*
             * Remove the flags which prevent collision.
             */
            object->m_isNoTouch = false;
            object->m_isDecoration = false;
            object->m_isDecoration2 = false;
            object->m_isPassable = false;

            /*
             * Enable Extended Collision for the object's
             * existing geometry.
             */
            object->m_hasExtendedCollision = true;

            /*
             * Recalculate the object's collision geometry.
             */
            object->setObjectRectDirty(true);
            object->setOrientedRectDirty(true);
            object->updateOrientedBox();

            changed++;
        }

        if (changed > 0) {
            FLAlertLayer::create(
                "Real Hitbox 2.1",
                "Collision enabled for selected objects.",
                "OK"
            )->show();
        }
    }
};
