#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;


// ============================================================
// PLAYLAYER
// ============================================================

class $modify(RealHitboxPlayLayer, PlayLayer) {
    void addObject(GameObject* object) {
        // Сначала штатно добавляем объект.
        PlayLayer::addObject(object);

        if (!object)
            return;

        /*
            m_hasExtendedCollision = property 511.

            Для декоративных объектов мы используем этот сохранённый
            флаг как маркер Real Hitbox.
        */
        if (!object->m_hasExtendedCollision)
            return;

        // Если это уже обычный solid-блок, ничего делать не нужно.
        if (object->m_objectType == GameObjectType::Solid)
            return;

        /*
            Добавляем ТОТ ЖЕ GameObject в настоящий список
            collision-блоков PlayLayer.

            В отличие от старого варианта:
            - новый GameObject не создаётся;
            - объект не меняется в LevelEditorLayer;
            - editor-объект остаётся декоративным;
            - изменение позиции/настроек в редакторе его не ломает.
        */

        bool alreadyExists = false;

        for (auto existing : m_solidCollisionObjects) {
            if (existing == object) {
                alreadyExists = true;
                break;
            }
        }

        if (alreadyExists)
            return;

        /*
            Это уже копия объекта внутри PlayLayer.
            Здесь можно изменить тип без риска сломать
            оригинальный объект в редакторе.
        */

        object->m_objectType = GameObjectType::Solid;

        // Обязательные параметры для нормальной коллизии.
        object->m_isNoTouch = false;
        object->m_isPassable = false;

        /*
            Не трогаем:
                m_isDecoration
                m_isDecoration2

            Поэтому визуально объект остаётся тем же самым.
        */

        object->setObjectRectDirty(true);
        object->setOrientedRectDirty(true);

        // Обновляем OBB.
        object->updateOrientedBox();

        // Добавляем именно этот объект в штатный список solid.
        m_solidCollisionObjects.push_back(object);

        m_solidCollisionObjectsCount =
            static_cast<int>(m_solidCollisionObjects.size());
    }
};


// ============================================================
// EDITOR
// ============================================================

class $modify(RealHitboxEditor, LevelEditorLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* hitboxButton = nullptr;
    };

    bool init(GJGameLevel* level, bool noUI) {
        if (!LevelEditorLayer::init(level, noUI))
            return false;

        // ----------------------------------------------------
        // Menu
        // ----------------------------------------------------

        auto menu = CCMenu::create();

        menu->setID("real-hitbox-menu");
        menu->setPosition({0.f, 0.f});

        // ----------------------------------------------------
        // Button
        // ----------------------------------------------------

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

        // ----------------------------------------------------
        // Version 2.1
        // ----------------------------------------------------

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


    // ========================================================
    // REAL HITBOX BUTTON
    // ========================================================

    void onRealHitbox(CCObject*) {
        if (!m_editorUI ||
            !m_editorUI->m_selectedObjects) {
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

        int changed = 0;

        for (auto object :
             CCArrayExt<GameObject*>(selected)) {

            if (!object)
                continue;

            /*
                Не трогаем обычные solid-блоки.

                Real Hitbox нужен именно для объектов,
                которые сами по себе не имеют обычной коллизии.
            */
            if (object->m_objectType ==
                GameObjectType::Solid) {
                continue;
            }

            /*
                Включаем/выключаем маркер.

                property 511 реально существует в GD 2.2.081
                как m_hasExtendedCollision.
            */
            object->m_hasExtendedCollision =
                !object->m_hasExtendedCollision;

            object->setObjectRectDirty(true);
            object->setOrientedRectDirty(true);

            changed++;
        }

        if (changed == 0) {
            FLAlertLayer::create(
                "Real Hitbox 2.1",
                "Select a decorative object.",
                "OK"
            )->show();
        }
    }
};
