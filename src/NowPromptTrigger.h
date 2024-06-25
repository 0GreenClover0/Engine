#pragma once

#include "AK/Badge.h"
#include "Component.h"

class NowPromptTrigger : public Component
{
public:
    static std::shared_ptr<NowPromptTrigger> create();
    explicit NowPromptTrigger(AK::Badge<NowPromptTrigger>);

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void awake() override;
    virtual void update() override;

private:
    std::weak_ptr<Entity> m_story_now_prompt = {};
    glm::vec3 m_now_prompt_pos = {1.543f, 2.0f, 2.731f};

    bool m_entered_triger = false;
    float m_time_of_showing_prompt = 0.0f;
};
