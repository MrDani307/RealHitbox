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

            // Create a real solid GD block.
            auto hitbox = this->createObject(
                1,
                source->getPosition(),
                false
            );

            if (!hitbox)
                continue;

            // Match the source transform.
            hitbox->setRotation(source->getRotation());
            hitbox->setFlipX(source->isFlipX());
            hitbox->setFlipY(source->isFlipY());

            // Calculate the source object's dimensions.
            auto size = source->getContentSize();

            float width = size.width * source->getScaleX();
            float height = size.height * source->getScaleY();

            // Normal solid block = 30x30 editor units.
            float sx = width / 30.f;
            float sy = height / 30.f;

            if (sx < 0.01f)
                sx = 0.01f;

            if (sy < 0.01f)
                sy = 0.01f;

            // Scale the collision block.
            static_cast<cocos2d::CCNode*>(hitbox)->setScale(
                sx,
                sy
            );

            // Hide the collision block visually.
            hitbox->setOpacity(0);

            // Update the editor.
            this->objectMoved(hitbox);
        }
    }
};
