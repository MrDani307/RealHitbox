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

        // Don't add our button when the editor is created without UI.
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

        // CCARRAY_FOREACH was removed in Geode 5.
        // CCArrayExt is the replacement.
        for (auto source : CCArrayExt<GameObject*>(selected)) {
            if (!source)
                continue;

            // Create a real Geometry Dash block through the editor.
            // This also properly registers it in LevelEditorLayer.
            auto hitbox = this->createObject(
                1,
                source->getPosition(),
                false
            );

            if (!hitbox)
                continue;

            hitbox->setRotation(source->getRotation());
            hitbox->setFlipX(source->isFlipX());
            hitbox->setFlipY(source->isFlipY());

            // A normal GD block is 30x30 editor units.
            // Match the source object's visible content bounds
            // using its current scale.
            auto size = source->getContentSize();

            float sx = (size.width * source->getScaleX()) / 30.f;
            float sy = (size.height * source->getScaleY()) / 30.f;

            if (sx <= 0.f)
                sx = 1.f;

            if (sy <= 0.f)
                sy = 1.f;

            // Explicitly call CCNode's setScale to avoid the
            // RealHitboxEditor::setScale name collision.
            static_cast<cocos2d::CCNode*>(hitbox)->setScale(sx, sy);

            // Hide the visual sprite while keeping the actual
            // Geometry Dash GameObject and its collision.
            hitbox->setOpacity(0);

            // Tell the editor that the object was modified.
            this->objectMoved(hitbox);
        }
    }
};
