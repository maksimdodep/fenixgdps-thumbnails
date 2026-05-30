#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <geode::utils::web>
#include <chrono>

using namespace geode::prelude;

std::string getThumbnailUrl(int levelID) {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    return fmt::format(
        "https://githubusercontent.com{}.png?t={}",
        levelID,
        millis
    );
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    CCSprite* m_bgSprite = nullptr;

    bool init(GJGameLevel * level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        int levelID = level->m_levelID.value();
        auto cachePath = Mod::get()->getLaurenceCacheDir() / fmt::format("bg_{}.png", levelID);

        web::AsyncWebRequest()
            .fetch(getThumbnailUrl(levelID))
            .into(cachePath)
            .then([this, cachePath](auto) {
            this->displayBackground(cachePath.string());
                })
            .expect([](std::string const&) {});

        this->scheduleUpdate();

        return true;
    }

    void displayBackground(std::string const& path) {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto sprite = CCSprite::create(path.c_str());
        if (!sprite) return;

        sprite->setPosition({ winSize.width / 2, winSize.height / 2 });

        float scaleX = (winSize.width / sprite->getContentSize().width) * 1.2f;
        float scaleY = (winSize.height / sprite->getContentSize().height) * 1.2f;
        sprite->setScaleX(scaleX);
        sprite->setScaleY(scaleY);

        sprite->setColor({ 90, 90, 90 });
        sprite->setOpacity(160);

        m_fields->m_bgSprite = sprite;
        this->addChild(sprite, -1);
    }

    void update(float dt) override {
        LevelInfoLayer::update(dt);

        if (!m_fields->m_bgSprite) return;

        auto director = CCDirector::sharedDirector();
        auto winSize = director->getWinSize();
        auto mousePos = director->getOpenGLView()->getMousePosition();
        auto convertedMousePos = director->convertToGL(mousePos);

        float offsetX = (convertedMousePos.x / winSize.width) - 0.5f;
        float offsetY = (convertedMousePos.y / winSize.height) - 0.5f;

        float maxMovement = 15.0f;

        float targetX = (winSize.width / 2) - (offsetX * maxMovement);
        float targetY = (winSize.height / 2) - (offsetY * maxMovement);

        float currentX = m_fields->m_bgSprite->getPositionX();
        float currentY = m_fields->m_bgSprite->getPositionY();

        m_fields->m_bgSprite->setPositionX(currentX + (targetX - currentX) * 0.1f);
        m_fields->m_bgSprite->setPositionY(currentY + (targetY - currentY) * 0.1f);
    }
};

class $modify(MyLevelCell, LevelCell) {
    bool init() {
        if (!LevelCell::init()) return false;

        if (!this->m_level) return true;
        int levelID = this->m_level->m_levelID.value();

        auto cachePath = Mod::get()->getLaurenceCacheDir() / fmt::format("thumb_{}.png", levelID);

        web::AsyncWebRequest()
            .fetch(getThumbnailUrl(levelID))
            .into(cachePath)
            .then([this, cachePath](auto) {
            this->displayThumbnail(cachePath.string());
                })
            .expect([](std::string const&) {});

        return true;
    }

    void displayThumbnail(std::string const& path) {
        auto sprite = CCSprite::create(path.c_str());
        if (!sprite) return;

        sprite->setPosition({ 345, 42 });
        sprite->setScale(0.35f);

        this->addChild(sprite);
    }
};
