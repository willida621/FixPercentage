#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
using namespace geode::prelude;

class $modify(PlayLayer) {
    struct Fields {
        float startPosPercent = 0.f;
        float startPosX = 0.f;
        float startPosTime = 0.f;
    };
    
    void resetLevel() {
        PlayLayer::resetLevel();
       
        if (!m_startPosObject) return;
        
        m_fields->startPosX = m_startPosObject->getPositionX();
        m_fields->startPosPercent = (m_fields->startPosX / m_levelLength) * 100.f;
        m_fields->startPosTime = this->timeForPos({m_fields->startPosX, 0.f}, 0.f, 0.f, true, 0.f);
    }
    
    float getCurrentPercent() {
        if (m_startPosObject && m_fields->startPosTime > 0.f) {
            const float totalLevelTime = this->timeForPos({m_levelLength, 0.f}, 0.f, 0.f, true, 0.f);
            const float adjustedTotalTime = totalLevelTime - m_fields->startPosTime;
            const float adjustedCurrentTime = m_gameState.m_levelTime - m_fields->startPosTime;
            
            if (adjustedTotalTime <= 0.f) return m_fields->startPosPercent;
            
            const float progressPercent = (adjustedCurrentTime / adjustedTotalTime) * 100.f;
            const float totalPercent = m_fields->startPosPercent + 
                progressPercent * ((100.f - m_fields->startPosPercent) / 100.f);
            
            return std::clamp(totalPercent, 0.f, 100.f);
        }

        return PlayLayer::getCurrentPercent();
    }
    
    void updateProgressbar() {
        if (!m_progressBar || !m_progressFill) return;
        
        const float percent = getCurrentPercent();
        const float width = (percent / 100.f) * m_progressWidth;
       
        m_progressFill->setScaleX(width / m_progressFill->getContentSize().width);
       
        if (m_percentageLabel) {
            m_percentageLabel->setString(
                m_decimalPercentage 
                    ? fmt::format("{:.1f}%", percent).c_str()
                    : fmt::format("{}%", static_cast<int>(percent)).c_str()
            );
        }
    }
};