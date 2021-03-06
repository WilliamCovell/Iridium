#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>

#define SSTR(x) static_cast<std::ostringstream&>((std::ostringstream()<<std::dec<<x)).str()
sf::RenderWindow win(sf::VideoMode(1080, 720), "Iridium");
int game_state = 0; //main menu 0, level select 1, game 2...
const double PI = 3.141592653589793238463;
sf::SoundBuffer B1;
sf::Sound S1;
sf::SoundBuffer B2;
sf::Sound S2;
sf::SoundBuffer B3;
sf::Sound S3;
sf::SoundBuffer B4;
sf::Sound S4;


sf::Vector2f AddVectors(int *points, int total_detection_points, double total_speed)
{
    sf::Vector2f vector_total(0.f, 0.f);
    for (int i = 0; i < points[0]; i++)
    {
        float angle = (((2 * points[i + 1]) * PI) / total_detection_points);
        sf::Vector2f v(-(cos(angle)), -(sin(angle)));
        vector_total += v;
    }
    float length = sqrt((vector_total.x * vector_total.x) + (vector_total.y * vector_total.y));
    sf::Vector2f u((vector_total.x / length) * total_speed, (vector_total.y / length) * total_speed);
    return u;
}

class Enemy
{
public:
    int start_x, start_y, end_x, end_y, speed_x, speed_y, radius, x, y;
    sf::CircleShape ball;
    Enemy(int startx, int starty, int endx, int endy, int speedx, int speedy, int pradius)
    {
        radius = pradius;
        start_x = startx - radius;
        start_y = starty - radius;
        x = start_x;
        y = start_y;
        end_x = endx - radius;
        end_y = endy - radius;
        speed_x = speedx;
        speed_y = speedy;
        ball.setRadius(radius);
        ball.setFillColor(sf::Color(255,0,0));
    }

    void Move()
    {
        if (speed_x != 0)
        {
            if (speed_x > 0 && x >= std::max(start_x, end_x))
            {
                speed_x *= -1;
            }
            else if (speed_x < 0 && x <= std::min(start_x, end_x))
            {
                 speed_x *= -1;
            }
        }
        if (speed_y != 0)
        {
            if (speed_y > 0 && y >= std::max(start_y, end_y))
            {
                speed_y *= -1;
            }
            else if (speed_y < 0 && y <= std::min(start_y, end_y))
            {
                 speed_y *= -1;
            }
        }
        x += speed_x;
        y += speed_y;
        ball.setPosition(x, y);
    }
};

class Collectable
{
public:
    bool collected;
    int x_pos;
    int y_pos;
    int radius;
    sf::CircleShape ball;
    Collectable(int x, int y)
    {
        radius = 10;
        x_pos = x - radius;
        y_pos = y - radius;
        collected = false;
        ball.setFillColor(sf::Color(64, 64, 64));
        ball.setRadius(radius);
        ball.setPosition(x_pos, y_pos);
    }

    bool IsColliding(int x, int y, int r)
    {
        if (sqrt(pow(abs((x + r) - (x_pos + radius)), 2) + pow(abs((y + r) - (y_pos + radius)), 2)) < (radius + r))
        {
            ball.setFillColor(sf::Color(0, 102, 0));
            if(collected == false)
            {
                S3.play();
            }
            collected = true;
            return true;
        }
        return false;
    }
};

class Portal
{
public:
    bool teleported;
    int x1_pos;
    int y1_pos;
    int x2_pos;
    int y2_pos;
    int radius;
    sf::CircleShape portal1;
    sf::CircleShape portal2;

    Portal(int x1, int y1, int x2, int y2)
    {
        radius = 20;
        x1_pos = x1 - radius;
        y1_pos = y1 - radius;
        x2_pos = x2 - radius;
        y2_pos = y2 - radius;
        teleported = false;
        portal1.setFillColor(sf::Color(0,128,255));
        portal1.setRadius(radius);
        portal1.setPosition(x1_pos, y1_pos);
        portal2.setFillColor(sf::Color(255,128,0));
        portal2.setRadius(radius);
        portal2.setPosition(x2_pos, y2_pos);
    }
    void teleport (float& x, float& y, bool portal) // portal 1 is true, portal 2 is false
    {
        if (teleported == false)
        {
            teleported = true;
            S4.play();
            if (portal){x = x2_pos; y = y2_pos;}
            else {x = x1_pos; y = y1_pos;}
        }
    }
    void IsColliding(float& x, float& y, int& r)
    {
        if (sqrt(pow(abs((x + r) - (x1_pos + radius)), 2) + pow(abs((y + r) - (y1_pos + radius)), 2)) < (r + radius))
        {
            teleport(x, y, true);
        }
        else if (sqrt(pow(abs((x + r) - (x2_pos + radius)), 2) + pow(abs((y + r) - (y2_pos + radius)), 2)) < (r + radius))
        {
            teleport(x, y, false);
        }
        else {teleported = false;}
    }
};

class LevelData
{
public:
    /*
    Level
    ballx, bally, ballr, corb1x, corb1y, corb2x, corb2y, corbr
    #collectibles, coll1x, coll1y, ...
    #enemys, startx, starty, endx, endy, speedx, speedy, radius ...
    #portals, portala1x, portala1y, portala1exitx, portala1exity, ...
    */
    std::vector<std::vector<int>> Levels;

    LevelData()
    {
        Levels.resize(51);
        Levels[1] = {180, 180, 15, 45, 45, 1035, 45, 30, 0, 0, 0};
        Levels[2] = {90, 630, 15, 45, 45, 1035, 45, 30, 5, 315, 225, 315, 495, 540, 360, 765, 225, 765, 495, 0, 0, 0, 0};
        Levels[3] = {165, 255, 15, 45, 45, 1035, 45, 30, 0, 0, 0};
        Levels[4] = {90, 630, 15, 45, 45, 1035, 45, 30, 3, 225, 195, 540, 195, 855, 195, 0, 0, 0};
        Levels[5] = {345, 435, 15, 45, 315, 1015, 315, 30, 6, 315, 158, 405, 158, 495, 158, 585, 158, 675, 158, 765, 158, 0, 0, 0, 0};
        Levels[6] = {90, 315, 15, 45, 45, 1035, 45, 30, 4, 360, 315, 720, 315, 360, 495, 720, 495, 0, 0, 0};
        Levels[7] = {540, 293, 15, 45, 45, 1035, 45, 30, 6, 90, 135, 990, 135, 360, 383, 720, 383, 90, 630, 990, 630, 0, 0, 0};
        Levels[8] = {};
        Levels[9] = {};
        Levels[10] = {};
        Levels[11] = {};
        Levels[12] = {90, 450, 15, 45, 45, 1035, 45, 30, 5, 315, 630, 405, 630, 495, 630, 585, 630, 675, 630, 0, 0, 0};
        Levels[13] = {90, 665, 15, 45, 45, 1035, 45, 30, 7, 405, 255, 495, 255, 345, 410, 390, 450, 450, 465, 510, 450, 555, 405, 0, 0, 0};
        Levels[14] = {};
        Levels[15] = {945, 158, 15, 45, 45, 1035, 45, 30, 4, 765, 158, 315, 158, 315, 563, 765, 563, 0, 0};
        Levels[16] = {90, 630, 15, 45, 45, 1035, 45, 30, 20, 473, 158, 608, 158, 473, 248, 608, 248, 473, 338, 608, 338, 473, 428, 608, 428, 383, 248, 698, 248, 293, 338, 383, 338, 698, 338, 788, 338, 203, 428, 293, 428, 383, 428, 698, 428, 788, 428, 878, 428, 0, 0, 0};
        Levels[17] = {};
        Levels[18] = {540, 405, 15, 45, 45, 1035, 45, 30, 2, 315, 360, 765, 360, 0, 0};
        Levels[19] = {};
        Levels[20] = {};
        Levels[21] = {90, 360, 15, 45, 45, 1035, 45, 30, 6, 338, 225, 338, 383, 338, 540, 743, 225, 743, 383, 743, 540, 1, 540, 225, 540, 540, 0, 3, 45, 0, 0, 0};
        Levels[22] = {60, 600, 15, 45, 45, 1035, 45, 30, 0, 3, 225, 45, 225, 675, 0, 5, 30, 480, 675, 480, 45, 0, 5, 30, 735, 45, 735, 675, 0, 5, 30, 0, 0};
        Levels[23] = {990, 135, 15, 45, 45, 1035, 45, 30, 0, 2, 390, 495, 870, 495, 3, 0, 60, 870, 630, 390, 630, 3, 0, 60, 0, 0};
        Levels[24] = {};
        Levels[25] = {};
        Levels[26] = {};
        Levels[27] = {};
        Levels[28] = {120, 645, 15, 45, 45, 1035, 45, 30, 0, 3, 45, 525, 915, 525, 2, 0, 30, 915, 405, 45, 405, 2, 0, 30, 45, 285, 915, 285, 2, 0, 30, 0, 0};
        Levels[29] = {};
        Levels[30] = {};
        Levels[31] = {};
        Levels[32] = {};
        Levels[33] = {};
        Levels[34] = {};
        Levels[35] = {};
        Levels[36] = {};
        Levels[37] = {};
        Levels[38] = {};
        Levels[39] = {};
        Levels[40] = {};
        Levels[41] = {135, 90, 15, 45, 45, 1035, 45, 30, 0, 0, 3, 135, 630, 405, 90, 405, 630, 675, 90, 675, 630, 945, 90, 0};
        Levels[42] = {};
        Levels[43] = {};
        Levels[44] = {};
        Levels[45] = {};
        Levels[46] = {};
        Levels[47] = {};
        Levels[48] = {};
        Levels[49] = {};
        Levels[50] = {};
    }
};

class Level
{
public:

    sf::Image level_image;
    sf::Texture level_texture;
    sf::Sprite level_sprite;
    sf::CircleShape control1;
    sf::CircleShape control2;
    int control_radius;
    LevelData* leveldata;
    int ballx;
    int bally;
    int ballr;
    int numCollectables, numEnemys, numPortals;
    Collectable* collectables[50];
    Portal* portals[50];
    Enemy* enemys[50];
    Level(int level_number)
    {
        leveldata = new LevelData();
        std::string file_path = "./resources/M" + SSTR(level_number) + ".png";
        level_texture.loadFromFile(file_path);
        level_image.loadFromFile(file_path);
        level_sprite.setTexture(level_texture, true);
        control_radius = leveldata->Levels[level_number][7];
        ballr = leveldata->Levels[level_number][2];
        ballx = leveldata->Levels[level_number][0] - ballr;
        bally = leveldata->Levels[level_number][1] - ballr;
        control1.setRadius(leveldata->Levels[level_number][7]);
        control2.setRadius(leveldata->Levels[level_number][7]);
        control1.setPosition(leveldata->Levels[level_number][3] - control_radius, leveldata->Levels[level_number][4] - control_radius);
        control2.setPosition(leveldata->Levels[level_number][5] - control_radius, leveldata->Levels[level_number][6] - control_radius);
        numCollectables = leveldata->Levels[level_number][8];
        for (int i = 0; i < numCollectables; i++)
        {
            collectables[i] = new Collectable(leveldata->Levels[level_number][(9 + (2 * i))], leveldata->Levels[level_number][(10 + (2 * i))]);
        }

        numEnemys = leveldata->Levels[level_number][(9 + (2 * numCollectables))];
        for (int i = 0; i < numEnemys; i++)
        {
            enemys[i] = new Enemy((leveldata->Levels[level_number][(10 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(11 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(12 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(13 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(14 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(15 + (2 * numCollectables) + (7 * i))]), (leveldata->Levels[level_number][(16 + (2 * numCollectables) + (7 * i))]));
        }

        numPortals = leveldata->Levels[level_number][(10 + (2 * numCollectables) + (7 * numEnemys))];
        for (int i = 0; i < numPortals; i++)
        {
            portals[i] = new Portal(leveldata->Levels[level_number][((10 + (2 * numCollectables) + 1 + (7 * numEnemys)) + (4 * i))], leveldata->Levels[level_number][((10 + (2 * numCollectables) + 2 + (7 * numEnemys)) + (4 * i))], leveldata->Levels[level_number][((10 + (2 * numCollectables) + 3 + (7 * numEnemys)) + (4 * i))], leveldata->Levels[level_number][((10 + (2 * numCollectables) + 4 + (7 * numEnemys)) + (4 * i))]);
        }
        std::cout << "Level: " << level_number << std::endl;
    };
    sf::Sprite GetSprite(){return level_sprite;}
    sf::Vector2f GetControlOrb1Position(void){return control1.getPosition();}
    sf::Vector2f GetControlOrb2Position(void){return control2.getPosition();}

    bool CheckEnding(int x, int y, int r)
    {
        if (level_image.getPixel((x + r), (y + r)) == sf::Color(128,0,128))
        {
            return true;
        }else{
            return false;
        }
    }

    void CollisionPoints(int *points, int x, int y, int r, int p)
    {
        int num_points = 0;
        sf::Color PointColour;
        for (int i = 0; i < p; i++)
        {
            PointColour = (level_image.getPixel(abs((x + r) + (r * cos(((2 * i) * PI) / p))), abs((y + r) + (r * sin(((2 * i) * PI) / p)))));
            if (PointColour == sf::Color::Black)
            {
                num_points++;
                points[num_points] = i;
            }
            if (PointColour == sf::Color::Red)
            {
                num_points = -1;
                points[0] = num_points;
                return;
            }
        }
        points[0] = num_points;
    }
    void EndCollisionPoints(int *points, int x, int y, int r, int p)
    {
        int num_points = 0;
        sf::Color PointColour;
        for (int i = 0; i < p; i++)
        {
            PointColour = (level_image.getPixel(abs((x + r) + (r * cos(((2 * i) * PI) / p))), abs((y + r) + (r * sin(((2 * i) * PI) / p)))));
            if (PointColour == sf::Color::Black || PointColour == sf::Color::Red)
            {
                num_points++;
                points[num_points] = i;
            }

        }
        points[0] = num_points;
    }
};

class Game
{
public:
    int current_level_num;
    int Quadrant_m1 = 1;
    int Quadrant_m2 = 1;
    int detection_points = 36;
    int detected_points[36 + 1];
    float x, y, prev_x, prev_y, gravity, x_speed, y_speed, pull_strength, pull_constant, i_friction_constant;
    bool pushpull = true;
    bool all_collected;
    sf::VertexArray lines;
    sf::Color colour_point;
    sf::CircleShape ball;
    sf::RectangleShape fade;
    int fade_alpha;
    bool increment_direction;
    int counter;
    Level* level;

    Game(sf::VertexArray line)
    {
        current_level_num = 1;
        level = new Level(current_level_num);
        x_speed = y_speed = 0.f;
        gravity = 0.07f;
        i_friction_constant = 0.35f;
        pull_constant = 0.00038f;
        lines = line;
        ball.setFillColor(sf::Color(0,128,0));
        ball.setRadius(level->ballr);
        x = level->ballx;
        y = level->bally;
        B1.loadFromFile("./resources/S1.ogg");
        S1.setBuffer(B1);
        B2.loadFromFile("./resources/S2.ogg");
        S2.setBuffer(B2);
        B3.loadFromFile("./resources/S3.ogg");
        S3.setBuffer(B3);
        B4.loadFromFile("./resources/S4.ogg");
        S4.setBuffer(B4);
        fade.setPosition(0,0);
        fade.setSize({win.getSize().x,win.getSize().y});
        fade.setFillColor(sf::Color(255,255,255,0));
    }

    void ResetLines()
    {
        level -> control1.setFillColor(sf::Color(128,128,128));
        level -> control2.setFillColor(sf::Color(128,128,128));
        lines[0].position = sf::Vector2f(0, 0); lines[0].color = sf::Color(0,128,0);
        lines[1].position = sf::Vector2f(0, 0); lines[1].color = sf::Color(0,128,0);
        lines[2].position = sf::Vector2f(0, 0); lines[2].color = sf::Color(0,128,0);
        lines[3].position = sf::Vector2f(0, 0); lines[3].color = sf::Color(0,128,0);
    }

    void RightOrb()
    {
        level -> control2.setFillColor(sf::Color(0,128,0));
        if ((x - level -> GetControlOrb2Position().x > 0) && (y - level -> GetControlOrb2Position().y <= 0)) {Quadrant_m1 = -1; Quadrant_m2 = 1;}       // Q1
            else if ((x - level -> GetControlOrb2Position().x <= 0) && (y - level -> GetControlOrb2Position().y < 0)) {Quadrant_m1 = 1; Quadrant_m2 = 1;}   // Q2
            else if ((x - level -> GetControlOrb2Position().x < 0) && (y - level -> GetControlOrb2Position().y >= 0)) {Quadrant_m1 = 1; Quadrant_m2 = -1;}  // Q3
            else {Quadrant_m1 = -1; Quadrant_m2 = -1;}                                                                                                      // Q4
            pull_strength = (sqrt(pow(fabs(((y + level -> ballr) - (level -> GetControlOrb2Position().y + level -> control_radius))), 2) + pow(fabs(((x + level -> control_radius) - (level -> GetControlOrb2Position().x + level -> control_radius))), 2))) * 0.9f;
            if (pushpull == true)
            {
                x_speed += (cos(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb2Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb2Position().x + level -> control_radius)))))) * Quadrant_m1 * ((pull_strength * pull_constant));
                y_speed += (sin(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb2Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb2Position().x + level -> control_radius)))))) * Quadrant_m2 * ((pull_strength * (pull_constant / 0.5f)));
            }else{
                x_speed += (cos(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb2Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb2Position().x + level -> control_radius)))))) * (1 / Quadrant_m1 * ((pull_strength * pull_constant) * -1));
                y_speed += (sin(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb2Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb2Position().x + level -> control_radius)))))) * (1 / Quadrant_m2 * ((pull_strength * pull_constant / 0.5f) * -1));
            }
            lines[2].position = sf::Vector2f(level -> GetControlOrb2Position().x + level -> control_radius, level -> GetControlOrb2Position().y + level -> control_radius);
            lines[3].position = sf::Vector2f((x + ball.getRadius()), (y + ball.getRadius()));
    }

    void LeftOrb()
    {
        level -> control1.setFillColor(sf::Color(0,128,0));
        if ((x - level -> GetControlOrb1Position().x > 0) && (y - level -> GetControlOrb1Position().y <= 0)) {Quadrant_m1 = -1; Quadrant_m2 = 1;}       // Q1
            else if ((x - level -> GetControlOrb1Position().x <= 0) && (y - level -> GetControlOrb1Position().y < 0)) {Quadrant_m1 = 1; Quadrant_m2 = 1;}   // Q2
            else if ((x - level -> GetControlOrb1Position().x < 0) && (y - level -> GetControlOrb1Position().y >= 0)) {Quadrant_m1 = 1; Quadrant_m2 = -1;}  // Q3
            else {Quadrant_m1 = -1; Quadrant_m2 = -1;}                                                                                                      // Q4
            pull_strength = (sqrt(pow(fabs(((y + level -> ballr) - (level -> GetControlOrb1Position().y + level -> control_radius))), 2) + pow(fabs(((x + level -> control_radius) - (level -> GetControlOrb1Position().x + level -> control_radius))), 2))) * 0.9f;
            if (pushpull == true)
            {
                x_speed += (cos(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb1Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb1Position().x + level -> control_radius)))))) * Quadrant_m1 * ((pull_strength * pull_constant));
                y_speed += (sin(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb1Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb1Position().x + level -> control_radius)))))) * Quadrant_m2 * ((pull_strength * (pull_constant / 0.5f)));
            }else{
                x_speed += (cos(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb1Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb1Position().x + level -> control_radius)))))) * (1 / Quadrant_m1 * ((pull_strength * pull_constant) * -1));
                y_speed += (sin(atan(fabs(((y + level -> ballr) - (level -> GetControlOrb1Position().y + level -> control_radius))) / fabs(((x + level -> ballr) - (level -> GetControlOrb1Position().x + level -> control_radius)))))) * (1 / Quadrant_m2 * ((pull_strength * pull_constant / 0.5f) * -1));
            }
            lines[0].position = sf::Vector2f(level -> GetControlOrb1Position().x + level -> control_radius, level -> GetControlOrb1Position().y + level -> control_radius);
            lines[1].position = sf::Vector2f((x + ball.getRadius()), (y + ball.getRadius()));
    }

    void SwitchPushPull(){pushpull = !pushpull;}

    void Movement()
    {
        if (pushpull){y_speed += gravity;}else{y_speed -= gravity;}
        prev_x = x;
        prev_y = y;
        y += y_speed;
        x += x_speed;
        level->CollisionPoints(detected_points, x, y, ball.getRadius(), detection_points);
        for (int i = 0; i < level->numCollectables; i++)
        {
            level->collectables[i]->IsColliding(x, y, level->ballr);
        }
        for (int i = 0; i < level->numPortals; i++)
        {
            level->portals[i]->IsColliding(x, y, level->ballr);
        }
        if (detected_points[0] != 0)
        {
            if (detected_points[0] > 0)
            {
                sf::Vector2f col_tangent = AddVectors(detected_points, detection_points, (sqrt((x_speed * x_speed) + (y_speed * y_speed))));
                x_speed += (col_tangent.x) * i_friction_constant;
                y_speed += (col_tangent.y) * i_friction_constant;
                x = prev_x;
                y = prev_y;
            }
            else // collided with obstacle
            {
                x = level->ballx;
                y = level->bally;
                x_speed = 0;
                y_speed = 0;
                S1.play();
                for(int j = 0; j < level->numCollectables; j++)
                {
                    level->collectables[j]->collected = false;
                    level->collectables[j]->ball.setFillColor(sf::Color(64, 64, 64));
                }
            }
        }
        colour_point = level->level_image.getPixel(x, y);
        if(colour_point == sf::Color(255,255,0)){pushpull = false;}
        if(colour_point == sf::Color(0,128,0)){pushpull = true;}
        if (level->CheckEnding(x, y, level -> ballr))
        {
            all_collected = true;
            for (int i = 0; i < level->numCollectables; i++)
            {
                if (level->collectables[i]->collected == false)
                {
                    all_collected = false;
                }
            }
            if (all_collected)
            {
                S2.play();
                fade_alpha = 0;
                increment_direction = true;
                counter = 0;
                while (true)
                {
                    if (increment_direction == true){fade_alpha += 5;}
                    else {fade_alpha -= 5;}
                    if (fade_alpha == 0){break;}
                    if (current_level_num == 50)
                    {
                        game_state = 0;
                        current_level_num = 1;
                        break;
                    }
                    if (fade_alpha == 255)
                    {
                        increment_direction = false;
                        while (counter < 20)
                        {
                            counter++;
                            win.draw(fade);
                            win.display();
                        }
                        while (true)
                        {
                            if (level -> leveldata -> Levels[current_level_num + 1].size() > 0)
                            {
                                current_level_num++;
                                level = new Level(current_level_num);
                                x = level->ballx;
                                y = level->bally;
                                level -> control1.setFillColor(sf::Color(128,128,128));
                                level -> control2.setFillColor(sf::Color(128,128,128));
                                x_speed = 0.f;
                                y_speed = 0.f;
                                ball.setRadius(level->ballr);
                                break;
                            }
                            else {
                                current_level_num++;
                                if (current_level_num > 50)
                                {
                                    game_state = 0;
                                    current_level_num = 0;
                                    return;
                                }
                            }
                        }
                    }
                    if (pushpull){y_speed += gravity;}else{y_speed -= gravity;}
                    prev_x = x;
                    prev_y = y;
                    y += y_speed;
                    x += x_speed;
                    level->EndCollisionPoints(detected_points, x, y, ball.getRadius(), detection_points);
                    if (detected_points[0] != 0)
                    {
                        if (detected_points[0] > 0)
                        {
                            sf::Vector2f col_tangent = AddVectors(detected_points, detection_points, (sqrt((x_speed * x_speed) + (y_speed * y_speed))));
                            x_speed += (col_tangent.x) * i_friction_constant;
                            y_speed += (col_tangent.y) * i_friction_constant;
                            x = prev_x;
                            y = prev_y;
                        }
                    }
                    for (int i = 0; i < level->numEnemys; i++){level->enemys[i]->Move();}
                    ball.setPosition(x, y);
                    fade.setFillColor(sf::Color(0,0,0,fade_alpha));
                    win.draw(level -> GetSprite());
                    for (int i = 0; i < level -> numCollectables; i++){win.draw(level -> collectables[i] -> ball);}
                    for (int i = 0; i < level -> numPortals; i++)
                    {
                        win.draw(level -> portals[i] -> portal1);
                        win.draw(level -> portals[i] -> portal2);
                    }
                    for (int i = 0; i < level -> numEnemys; i++){win.draw(level -> enemys[i] -> ball);}
                    win.draw(level -> control1);
                    win.draw(level -> control2);
                    win.draw(ball);
                    win.draw(fade);
                    win.display();
                }
            }
        }
        for (int i = 0; i < level->numEnemys; i++)
        {
            level->enemys[i]->Move();
            if (sqrt(pow(abs((x + level->ballr) - (level->enemys[i]->x + level->enemys[i]->radius)), 2) + pow(abs((y + level->ballr) - (level->enemys[i]->y + level->enemys[i]->radius)), 2)) < (level->ballr + level->enemys[i]->radius))
            {
                x = level->ballx;
                y = level->bally;
                x_speed = 0;
                y_speed = 0;
                S1.play();
                for(int j = 0; j < level->numCollectables; j++)
                {
                    level->collectables[j]->collected = false;
                    level->collectables[j]->ball.setFillColor(sf::Color(64, 64, 64));
                }
            }
        }
        ball.setPosition(x, y);
    }
};

int main()
{
    win.setFramerateLimit(80);
    win.setKeyRepeatEnabled(false);
    sf::Event event;
    sf::VertexArray lines(sf::Lines, 4);
    bool keyState[sf::Keyboard::KeyCount];
    for(int i = 0; i < (sf::Keyboard::KeyCount); i++){keyState[i] = false;}
    Game* game = new Game(lines);

    sf::Image MainMenu_image;
    MainMenu_image.loadFromFile("./resources/MainMenu.png");
    sf::Texture MainMenu_texture;
    MainMenu_texture.loadFromFile("./resources/MainMenu.png");
    sf::Sprite MainMenu_sprite;
    MainMenu_sprite.setTexture(MainMenu_texture, true);

    sf::Image LevelSelect_image;
    sf::Texture LevelSelect_texture;
    sf::Sprite LevelSelect_sprite;

    std::vector <std::vector <int>> SelectionBoxData;
    SelectionBoxData.resize(4);
    SelectionBoxData[0] = {465, 267, 150, 75}; // Play
    SelectionBoxData[1] = {420, 365, 240, 75}; // Options
    SelectionBoxData[2] = {465, 465, 150, 75}; // Help
    SelectionBoxData[3] = {465, 555, 150, 75}; // Exit

    sf::RectangleShape SelectionBox;
    int CurrentSelection = 0;
    SelectionBox.setPosition(SelectionBoxData[CurrentSelection][0],SelectionBoxData[CurrentSelection][1]);
    SelectionBox.setSize({SelectionBoxData[CurrentSelection][2],SelectionBoxData[CurrentSelection][3]});
    SelectionBox.setFillColor(sf::Color::Transparent);
    SelectionBox.setOutlineColor(sf::Color(0,128,0));
    SelectionBox.setOutlineThickness(5.f);

    while (win.isOpen())
    {
        win.clear();
        if (game_state == 0) // Main Menu
        {
            while(win.pollEvent(event))
            {
                switch(event.type)
                {
                case sf::Event::Closed:
                    win.close();
                    break;
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Escape) {win.close();}
                    if (event.key.code == sf::Keyboard::Up && !keyState[event.key.code])
                    {
                        S3.play();
                        if (CurrentSelection == 0) {CurrentSelection = 3;}
                        else {CurrentSelection --;}
                    }
                    if (event.key.code == sf::Keyboard::Down && !keyState[event.key.code])
                    {
                        S3.play();
                        if (CurrentSelection == 3) {CurrentSelection = 0;}
                        else {CurrentSelection ++;}
                    }
                    if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)
                    {
                        if (CurrentSelection == 3) {win.close();}
                        else {game_state = 2; S2.play();}

                        //else {game_state = CurrentSelection + 2; S2.play();}
                    }
                    break;
                default:
                    break;
                }
            }
            SelectionBox.setPosition(SelectionBoxData[CurrentSelection][0],SelectionBoxData[CurrentSelection][1]);
            SelectionBox.setSize({SelectionBoxData[CurrentSelection][2],SelectionBoxData[CurrentSelection][3]});
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (sf::Mouse::getPosition(win).x >= SelectionBoxData[3][0] && sf::Mouse::getPosition(win).x <= SelectionBoxData[3][0] + SelectionBoxData[3][2]  && sf::Mouse::getPosition(win).y >= SelectionBoxData[3][1] && sf::Mouse::getPosition(win).y <= SelectionBoxData[3][1] + SelectionBoxData[3][3])
                {
                    win.close();
                }
                else {
                    for (int i = 0; i < 3; i++)
                    {
                        if (sf::Mouse::getPosition(win).x >= SelectionBoxData[i][0] && sf::Mouse::getPosition(win).x <= SelectionBoxData[i][0] + SelectionBoxData[i][2]  && sf::Mouse::getPosition(win).y >= SelectionBoxData[i][1] && sf::Mouse::getPosition(win).y <= SelectionBoxData[i][1] + SelectionBoxData[i][3])
                        {
                            game_state = i + 2;
                        }
                    }
                }
            }
            win.draw(MainMenu_sprite);
            win.draw(SelectionBox);
        }
        else if (game_state == 1) // Level Select
        {
            game_state = 0;
        }
        else if (game_state == 2) // Game
        {
            game->ResetLines();
            game->Movement();
            while(win.pollEvent(event))
            {
                switch(event.type)
                {
                    case sf::Event::Closed:
                        win.close();
                        break;
                    case sf::Event::KeyPressed:
                        if (event.key.code == sf::Keyboard::Escape) {win.close();}
                        //if (event.key.code == sf::Keyboard::Space && !keyState[event.key.code])
                            //{game->SwitchPushPull();}
                        if (event.key.code == sf::Keyboard::R && !keyState[event.key.code])
                        {
                            game->level = new Level(game->current_level_num);
                            game->x = game->level->ballx;
                            game->y = game->level->bally;
                            game->x_speed = game->y_speed = 0;
                            game->pushpull = true;
                        }
                        //if (event.key.code == sf::Keyboard::R && !keyState[event.key.code]) {current_level_num++; level = new Level(current_level_num, 30, 30, 1000, 30, sf::Color::Red);}
                        keyState[event.key.code] = true;
                        break;
                    case sf::Event::KeyReleased:
                        keyState[event.key.code] = false;
                        break;
                    default:
                        break;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Mouse::isButtonPressed(sf::Mouse::Right)){game->LeftOrb();}
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Mouse::isButtonPressed(sf::Mouse::Left)){game->RightOrb();}
            win.draw(game->level->GetSprite());
            for (int i = 0; i < game->level->numCollectables; i++){win.draw(game->level->collectables[i]->ball);}
            for (int i = 0; i < game->level->numPortals; i++)
            {
                win.draw(game->level->portals[i]->portal1);
                win.draw(game->level->portals[i]->portal2);
            }
            for (int i = 0; i < game->level->numEnemys; i++){win.draw(game->level->enemys[i]->ball);}
            win.draw(game->lines);
            win.draw(game->ball);
            win.draw(game->level -> control1);
            win.draw(game->level -> control2);
        }
        win.display();
    }
    return 0;
}
