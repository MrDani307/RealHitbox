#include <Geode/Geode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>

using namespace geode::prelude;

class $modify(RealHitboxEditor, LevelEditorLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* hitboxButton = nullptr;
    };

    bool init(GJGameLevel* level) {
        if (!LevelEditorLayer::init(level))
            return false;

        auto menu = CCMenu::create();
        menu->setID("real-hitbox-menu");
        menu->setPosition({0.f, 0.f});

        auto sprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        if (!sprite)
            sprite = CCSprite::create();

        sprite->setScale(0.55f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(RealHitboxEditor::onCreateBlockHitbox)
        );

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

        CCObject* item = nullptr;

        CCARRAY_FOREACH(selected, item) {
            auto source = typeinfo_cast<GameObject*>(item);
            if (!source)
                continue;

            // Geometry Dash block object.
            // The object itself supplies the real GD collision.
            auto hitbox = GameObject::createWithKey(1);
            if (!hitbox)
                continue;

            hitbox->setPosition(source->getPosition());
            hitbox->setRotation(source->getRotation());
            hitbox->setFlipX(source->isFlipX());
            hitbox->setFlipY(source->isFlipY());

            // A normal GD block is 30x30 in editor units.
            // Match the source object's visible content bounds as closely
            // as possible using the source scale.
            auto size = source->getContentSize();

            float sx = (size.width * source->getScaleX()) / 30.f;
            float sy = (size.height * source->getScaleY()) / 30.f;

            if (sx <= 0.f) sx = 1.f;
            if (sy <= 0.f) sy = 1.f;

            hitbox->setScale(sx, sy);

            // Keep it as a real GameObject but hide its sprite.
            // Collision is still provided by the actual GD object.
            hitbox->setOpacity(0);

            this->addObject(hitbox);
        }
    }
};
