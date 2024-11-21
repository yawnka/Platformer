#include "Scene.h"

#define JUMPER_JUMP_POWER 2.0f

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 1;
    
    ~LevelB();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};
