#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
using namespace geode::prelude;

class $modify(PlayLayer)
{
    struct Fields
    {
        float startPosPercent = 0.f;
        float startPosX = 0.f;
        float startPosTime = 0.f;
        float fromPercentage = 0.f;
        float lastFrom = 0.f;
        bool m_decimalPercentage = false;
    };

    bool decimalEnabled()
    {
        return !!Mod::get()->getSettingValue<bool>("decimal-percentages");
    }

    bool showFromEnabled()
    {
        return !!Mod::get()->getSettingValue<bool>("show-from-percentages");
    }

    void resetLevel()
    {
        PlayLayer::resetLevel();

        const auto currentPercent = PlayLayer::getCurrentPercent();
        m_fields->fromPercentage = currentPercent;

        if (!m_startPosObject)
            return;

        m_fields->startPosX = m_startPosObject->getPositionX();
        m_fields->startPosPercent = (m_fields->startPosX / m_levelLength) * 100.f;
        m_fields->startPosTime = this->timeForPos({m_fields->startPosX, 0.f}, 0.f, 0.f, true, 0.f);
    }

    float getCurrentPercent()
    {
        const auto selfResult = PlayLayer::getCurrentPercent();

        if (m_startPosObject && m_fields->startPosTime > 0.f)
        {
            const float totalLevelTime = this->timeForPos({m_levelLength, 0.f}, 0.f, 0.f, true, 0.f);
            const float adjustedTotalTime = totalLevelTime - m_fields->startPosTime;
            const float adjustedCurrentTime = m_gameState.m_levelTime - m_fields->startPosTime;

            if (adjustedTotalTime <= 0.f)
                return m_fields->startPosPercent;

            const float progressPercent = (adjustedCurrentTime / adjustedTotalTime) * 100.f;
            const float totalPercent = m_fields->startPosPercent +
                                       progressPercent * ((100.f - m_fields->startPosPercent) / 100.f);

            const float newPercentage = std::clamp(totalPercent, 0.f, 100.f);

            // Fix when start-pos has not had time to load and
            // 0 is already written to the "fromPercentage"
            m_fields->fromPercentage = m_fields->startPosPercent;

            m_fields->lastFrom = newPercentage;
            return newPercentage;
        }

        return selfResult;
    }

    void updateProgressbar()
    {
        // You need to call original
        PlayLayer::updateProgressbar();

        if (!m_progressBar || !m_progressFill)
            return;

        const float percent = getCurrentPercent();
        const float width = (percent / 100.f) * m_progressWidth;

        // Replace setScaleX with setContentWidth
        m_progressFill->setContentWidth(width / m_progressFill->getContentSize().width);
        customUpdatePercentages(percent);
    }

    void customUpdatePercentages(float percent)
    {
        if (!m_percentageLabel)
            return;

        const auto decimal = decimalEnabled();
        const auto showFrom = showFromEnabled();

        std::string current = "";

        if (m_fields->fromPercentage > 0 && showFrom)
            current = decimal
                          ? fmt::format("{:.2f}%", m_fields->fromPercentage)
                          : fmt::format("{}%", static_cast<int>(m_fields->fromPercentage));

        // Format the new percentage
        std::string newPercent = decimal
                                     ? fmt::format("{:.2f}%", percent)
                                     : fmt::format("{}%", static_cast<int>(percent));

        // Append new percentage after fromPercentage if current is not empty
        if (!current.empty())
            current += "-" + newPercent;
        else
            current = newPercent;

        m_percentageLabel->setString(current.c_str());
    }
};