#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(BufferPlayer, PlayerObject) {
    struct Fields {
        float timeLeft = 0.f;
        bool pending = false;
        bool held = false;
        bool synthetic = false;
        bool releaseNext = false;
    };

    bool holdMode() {
        return m_isShip || m_isBird || m_isDart || m_isSwing;
    }

    bool pushButton(PlayerButton btn) {
        if (btn != PlayerButton::Jump || m_fields->synthetic || !PlayLayer::get()) {
            return PlayerObject::pushButton(btn);
        }
        m_fields->held = true;
        if (!m_isOnGround && !this->holdMode()) {
            auto ms = Mod::get()->getSettingValue<int64_t>("buffer-ms");
            if (ms > 0) {
                m_fields->pending = true;
                m_fields->timeLeft = static_cast<float>(ms) / 1000.f;
            }
        }
        return PlayerObject::pushButton(btn);
    }

    bool releaseButton(PlayerButton btn) {
        if (btn == PlayerButton::Jump && !m_fields->synthetic) {
            m_fields->held = false;
        }
        return PlayerObject::releaseButton(btn);
    }

    void update(float dt) {
        if (m_fields->releaseNext) {
            m_fields->releaseNext = false;
            m_fields->synthetic = true;
            PlayerObject::releaseButton(PlayerButton::Jump);
            m_fields->synthetic = false;
        }

        PlayerObject::update(dt);

        if (!m_fields->pending) return;

        m_fields->timeLeft -= dt;
        if (m_isOnGround) {
            m_fields->pending = false;
            if (!m_fields->held) {
                m_fields->synthetic = true;
                PlayerObject::pushButton(PlayerButton::Jump);
                m_fields->synthetic = false;
                m_fields->releaseNext = true;
            }
        } else if (m_fields->timeLeft <= 0.f) {
            m_fields->pending = false;
        }
    }
};
