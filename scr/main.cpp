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
                "Real Hitbox",
                "Select one or more objects first.",
                "OK"
            )->show();

            return;
        }

        for (auto source : CCArrayExt<GameObject*>(selected)) {
            if (!source)
                continue;

            /*
             * Create a real Geometry Dash block.
             *
             * Object ID 1 is a normal solid block and therefore
             * provides actual player collision.
             */
            auto hitbox = this->createObject(
                1,
                source->getPosition(),
                false
            );

            if (!hitbox)
                continue;

            /*
             * Match the source object's transform.
             */
            hitbox->setRotation(source->getRotation());
            hitbox->setFlipX(source->isFlipX());
            hitbox->setFlipY(source->isFlipY());

            /*
             * Calculate the size of the selected object.
             *
             * A normal block is approximately 30x30 editor units.
             */
            auto size = source->getContentSize();

            float width =
                size.width * source->getScaleX();

            float height =
                size.height * source->getScaleY();

            float sx = width / 30.f;
            float sy = height / 30.f;

            if (sx < 0.01f)
                sx = 0.01f;

            if (sy < 0.01f)
                sy = 0.01f;

            /*
             * Scale the collision block.
             */
            static_cast<cocos2d::CCNode*>(hitbox)->setScale(
                sx,
                sy
            );

            /*
             * Extended Collision makes the collision box follow
             * large object scaling more accurately.
             *
             * 101 is the Geometry Dash object property used for
             * Extended Collision.
             */
            hitbox->m_editorProperties[101] = 1;

            /*
             * Hide the actual block visually.
             * The GameObject itself remains in the level, so
             * its collision remains active during gameplay.
             */
            hitbox->setOpacity(0);

            /*
             * Keep the generated object synchronized with
             * the editor.
             */
            this->objectMoved(hitbox);
        }
    }
};
