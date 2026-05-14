#include <raylib.h>
#include <math.h>
#include <raymath.h>
#include <iostream>
#include <vector>
#include <cmath>

namespace PARAMETERS
{
    const int screenWIDTH = 1920;
    const int screenHEIGHT = 1080;
    const Color backgroundColor = {26, 27, 28, 255};
} // namespace PARAMETERS

using namespace std;

class Bird
{
private:
    // Vizualni parametri
    int WIDTH;
    int HEIGHT;

    Color COLOR;

    // Trenutne koordinate
    Vector2 position;

    // Sile kretanja
    Vector2 moveDir;   // Smjer u kojem gleda
    Vector2 moveForce; // Smjer prema kojem teži

    // brzina
    float speed;

public:
    Bird(Vector2 pos, Vector2 dir, Color col)
    {
        // Vizualni parametri
        WIDTH = 15;
        HEIGHT = 20;
        COLOR = col;

        // Trenutne koordinate
        position = pos;

        // Sile kretanja
        moveDir = dir;      // Smjer u kojem gleda
        moveForce = {0, 0}; // Smjer prema kojem teži

        // brzina
        speed = 10;
    }

    // Getteri
    Vector2 getPosition() { return position; }
    Vector2 getMoveDir() { return moveDir; }

    // Setteri
    void setMoveForce(Vector2 mf) { moveForce = mf; }

    void UpdatePosition() // Updatea poziciju ptice nad kojom je pozvana, ovisno o trenutnoj poziciji i smjeru kretanja
    {
        moveDir = moveDir + moveForce;
        moveDir = Vector2Normalize(moveDir);

        Vector2 newPos = position + moveDir * speed;

        // Teleportira na drugu stranu ekrana
        if (newPos.x > GetScreenWidth() + HEIGHT)
            newPos.x = 0;
        if (newPos.y > GetScreenHeight() + HEIGHT)
            newPos.y = 0;
        if (newPos.x < 0)
            newPos.x = GetScreenWidth();
        if (newPos.y < 0)
            newPos.y = GetScreenHeight();

        position = newPos;
    }

    void Draw() // Crta trokut ptice nad kojom je pozvana, ovisno o poziciji i trenutnom smjeru kretanja
    {
        if (moveDir == Vector2Zero())
            moveDir = {0, 1};

        moveDir = Vector2Normalize(moveDir);

        // Varijable tocaka
        Vector2 tip;
        Vector2 lVert;
        Vector2 rVert;

        tip.x = position.x + ((HEIGHT * 3 / 4) * moveDir.x);
        tip.y = position.y + ((HEIGHT * 3 / 4) * moveDir.y);

        rVert.x = position.x - ((HEIGHT / 4) * moveDir.x) + ((WIDTH / 2) * moveDir.y);
        rVert.y = position.y - ((HEIGHT / 4) * moveDir.y) - ((WIDTH / 2) * moveDir.x);

        lVert.x = position.x - ((HEIGHT / 4) * moveDir.x) - ((WIDTH / 2) * moveDir.y);
        lVert.y = position.y - ((HEIGHT / 4) * moveDir.y) + ((WIDTH / 2) * moveDir.x);

        DrawTriangle(rVert, lVert, tip, COLOR);
    }
};

Vector2
RandomPosition() // Nasumicna pozicija unutar pravokutnika prozora
{
    float x = rand() / (float)RAND_MAX * PARAMETERS::screenWIDTH;
    float y = rand() / (float)RAND_MAX * PARAMETERS::screenHEIGHT;

    return {x, y};
}
Vector2 RandMoveDir() // Nasumicni smjer na jedinicnoj kruznici
{
    float randomNumCircle = (rand() / (float)RAND_MAX) * 2.f * PI;
    Vector2 direction = {sin(randomNumCircle), cos(randomNumCircle)};

    return direction;
}

Color RandColor(Color startColor) // Nasumicna varijacije boje u svjetlosti
{
    float variationDeg = rand() / (float)RAND_MAX * 0.5 - 0.5;

    return ColorBrightness(startColor, variationDeg);
}

void CalculateForces(vector<Bird> &birds)
{
    // Namijesti radius
    int checkRadius = 150;

    // Namjesti tezine za pravila
    float separationWeight = 1;
    float alignmentWeight = 1;
    float cohesionWeight = 1;
    // Za odbijanje od cusora
    float mouseWeight = 0.7;

    int birdAmount = birds.size();

    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < birdAmount; i++)
    {
        Vector2 separationForce = Vector2Zero();
        Vector2 alignmentForce = Vector2Zero();
        Vector2 cohesionForce = Vector2Zero();
        Vector2 mouseForce = Vector2Zero();

        // Ptica za koju se racunaju sile
        Bird &currentBird = birds[i];
        Vector2 currentPos = currentBird.getPosition();

        int birdsInArea = 0;
        float sumX = 0;
        float sumY = 0;

        for (int j = 0; j < birdAmount; j++)
        {
            if (i == j)
                continue; // Preskoci ako se radi o istoj ptici

            // Ptica u okolici currentBird
            Bird &otherBird = birds[j];
            Vector2 otherPos = otherBird.getPosition();

            // 1. Provjeri da je otherBird u kvadratu oko currentBird
            if (otherPos.x > currentPos.x + checkRadius)
                continue;
            if (otherPos.x < currentPos.x - checkRadius)
                continue;
            if (otherPos.y > currentPos.y + checkRadius)
                continue;
            if (otherPos.y < currentPos.y - checkRadius)
                continue;

            // 2. Provjeri da je otherBird u krugu oko currentBird
            float birdDistance = Vector2Distance(currentPos, otherPos);
            if (birdDistance > checkRadius)
                continue;

            birdsInArea++; // Ako je kod stigao do tu znaci da je otherBird u okolici

            // ---------- SEPARATION ----------
            Vector2 posDifference = currentPos - otherPos;

            float distance = Vector2Length(posDifference);
            if (distance > 0.1f)
            {
                separationForce += posDifference / distance;
            }

            // ---------- ALIGNMENT ----------
            alignmentForce += otherBird.getMoveDir();

            // ---------- COHESION ----------
            sumX += otherPos.x;
            sumY += otherPos.y;
        }
        // Provjeri lokaciju cursora
        float mouseDistance = Vector2Distance(currentPos, mousePos);
        if (mouseDistance < checkRadius)
        {
            mouseForce = (currentPos - mousePos) / mouseDistance;
        }

        // Normaliziraj vektore
        if (birdsInArea > 0) // Ako nema ptica u okolici
        {
            // Izracunaj prosjek do kraja
            Vector2 cohesionPoint = {sumX / (float)birdsInArea, sumY / (float)birdsInArea};
            cohesionForce = cohesionPoint - currentPos;

            cohesionForce = Vector2Normalize(cohesionForce);
        }
        if (Vector2Length(separationForce) > 0)
        {
            separationForce = Vector2Normalize(separationForce);
        }
        if (Vector2Length(alignmentForce) > 0)
        {
            alignmentForce = Vector2Normalize(alignmentForce);
        }

        // Updateaj sile ptici
        birds[i].setMoveForce(
            separationForce * separationWeight +
            alignmentForce * alignmentWeight +
            cohesionForce * cohesionWeight +
            mouseForce * mouseWeight);
    }
}

int main()
{
    InitWindow(PARAMETERS::screenWIDTH, PARAMETERS::screenHEIGHT, "Boids");
    SetTargetFPS(60);

    int birdAmount = 500;
    vector<Bird> birds;

    // Initialisation
    for (int i = 0; i < birdAmount; i++)
    {
        birds.push_back(Bird(RandomPosition(), RandMoveDir(), RandColor({59, 111, 209, 255})));
    }

    while (!WindowShouldClose())
    {
        // 1. Event Handling
        CalculateForces(birds);

        // 2. Updating Positions
        for (int i = 0; i < birdAmount; i++)
        {
            birds[i].UpdatePosition();
        }

        // 3. Drawing
        BeginDrawing();
        ClearBackground(PARAMETERS::backgroundColor);

        for (int i = 0; i < birdAmount; i++)
        {
            birds[i].Draw();
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}