#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <string>
#include <iostream>

using namespace sf;

struct Enemy
{
    Sprite sprite;
    float speed;
    int type;

    Enemy(const sf::Texture& tex, int t, float s)
        : sprite(tex), speed(s), type(t)
    {
    }
};

struct Explosion
{
    sf::CircleShape shape;
    float life;
};

enum class GameState
{
    Intro
};

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Space War");
    window.setFramerateLimit(60);

    sf::Image icon;
    if (!icon.loadFromFile("logo.png"))
        return -1;

    window.setIcon(icon);

    //Texture
    Texture playerTexture, enemy1Tex, enemy2Tex;
    Texture introTexture, startBtnTexture;
    if (!playerTexture.loadFromFile("player.png") ||
        !enemy1Tex.loadFromFile("enemy1.png") ||
        !enemy2Tex.loadFromFile("enemy3.png") ||
        !introTexture.loadFromFile("intro.png") ||
        !startBtnTexture.loadFromFile("start_button.png"))
        return -1;

    std::vector<sf::Texture> enemyTextures;
    enemyTextures.push_back(enemy1Tex);
    enemyTextures.push_back(enemy2Tex);

    sf::Font font;
    if (!font.openFromFile("ARIALLGTITL.ttf"))
        return -1;

    Sprite introSprite(introTexture);
    introSprite.setScale({
800.f / introTexture.getSize().x,
600.f / introTexture.getSize().y
        });
    sf::Sprite startButton(startBtnTexture);
    sf::FloatRect bounds = startButton.getLocalBounds();
    startButton.setOrigin({
        bounds.position.x + bounds.size.x / 5.f,
        bounds.position.y + bounds.size.y / 2.f
        });
    startButton.setPosition({ 300.f, 480.f });
    startButton.setScale({ 0.5f, 0.5f });

    bool showIntro = true;

    Sprite player(playerTexture);
    player.setScale({ 0.2f, 0.1f });
    player.setPosition({ 380.f, 520.f });

    int lives = 3;
    int score = 0;
    float playerspeed = 6.f;

    std::vector<Enemy> enemies;

    auto spawnEnemies = [&]()
        {
            enemies.clear();
            for (int i = 0; i < 4; i++)
            {
                int type = std::rand() % enemyTextures.size();
                float speed = 3.f + static_cast<float>(std::rand() % 3);

                Enemy e(enemyTextures[type], type, speed);
                e.sprite.setScale({ 2.0f, 2.f });
                e.sprite.setPosition({
                    static_cast<float>(std::rand() % 760),
                    static_cast<float>(-(i * 200))
                    });
                enemies.push_back(e);
            }
        };
    spawnEnemies();

    std::vector<RectangleShape>bullets;
    Clock fireClock;

    bool powerMode = false;
    Clock powerClock;

    //stars
    std::vector<sf::CircleShape> stars;
    for (int i = 0; i < 100; i++)
    {
        sf::CircleShape star(1);
        star.setFillColor(sf::Color::White);
        star.setPosition({ (float)(std::rand() % 800), (float)(std::rand() % 600) });
        stars.push_back(star);
    }

    //explosions
    std::vector<Explosion> explosions;

    sf::Text scoreText(font);;
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 650.f, 10.f });

    sf::Text gameOverText(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(48);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setStyle(sf::Text::Bold);
    gameOverText.setOutlineColor(sf::Color::Black);
    gameOverText.setOutlineThickness(4.f);
    gameOverText.setPosition({ 180.f, 200.f });

    sf::Text restartText(font);
    restartText.setString("(R)Restart\n(ERC)Exit");
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition({ 270.f, 400.f });

    sf::Text finalScoreText(font);
    finalScoreText.setCharacterSize(36);
    finalScoreText.setFillColor(sf::Color::White);
    finalScoreText.setPosition({ 260.f, 320.f });

    GameState gameState = GameState::Intro;
    bool gameOver = false;
    Clock hitClock;
    bool hit = true;

    //loop
    while (window.isOpen())
    {
        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (showIntro)
        {
            sf::Vector2f mousePos = window.mapPixelToCoords(
                sf::Mouse::getPosition(window)
            );
            if (startButton.getGlobalBounds().contains(mousePos))
                startButton.setScale({ 0.55f, 0.55f });
            else
                startButton.setScale({ 0.5f, 0.5f });

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
                startButton.getGlobalBounds().contains(mousePos))
            {
                showIntro = false;
            }

            window.clear();
            window.draw(introSprite);
            window.draw(startButton);
            window.display();
            continue;
        }


        if (gameOver)
        {
            if (Keyboard::isKeyPressed(Keyboard::Scan::R))
            {
                lives = 3;
                score = 0;
                bullets.clear();
                explosions.clear();
                player.setPosition({ 380.f, 520.f });
                spawnEnemies();
                gameOver = false;
            }
            if (Keyboard::isKeyPressed(Keyboard::Scan::Escape))
                window.close();
        }

        if (!gameOver)
        {
            //player movement
            FloatRect pBounds = player.getGlobalBounds();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A) && pBounds.position.x > 0)
                player.move({ -playerspeed, 0.f });

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D) && pBounds.position.x + pBounds.size.x < 760)
                player.move({ playerspeed, 0.f });
            if (sf::Keyboard::isKeyPressed(Keyboard::Scan::W) && pBounds.position.y > 0)
                player.move({ 0.f, -playerspeed });
            if (sf::Keyboard::isKeyPressed(Keyboard::Scan::S) && pBounds.position.y + pBounds.size.y < 560)
                player.move({ 0.f, playerspeed });

            if (sf::Keyboard::isKeyPressed(Keyboard::Scan::LShift))
            {
                powerMode = true;
                powerClock.restart();
            }
            if (powerMode && powerClock.getElapsedTime().asSeconds() > 5.f)
                powerMode = false;

            //shooting
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && fireClock.getElapsedTime().asMilliseconds() > 250)
            {
                FloatRect p = player.getGlobalBounds();

                float offset = p.size.x * 0.25f;


                //left bullet
                sf::RectangleShape bulletL({ 4.f, 15.f });
                bulletL.setFillColor(sf::Color::Yellow);
                bulletL.setPosition({
                    p.position.x + offset - bulletL.getSize().x / 2.f,
                    p.position.y
                    });

                //right bullet
                sf::RectangleShape bulletR({ 4.f, 15.f });
                bulletR.setFillColor(sf::Color::Yellow);
                bulletR.setPosition({
                    p.position.x + p.size.x - offset - bulletR.getSize().x / 2.f,
                    p.position.y
                    });

                bullets.push_back(bulletL);
                bullets.push_back(bulletR);

                if (powerMode)
                {
                    bullets[bullets.size() - 1].setSize({ 8.f, 25.f });
                    bullets[bullets.size() - 2].setSize({ 8.f, 25.f });
                    bullets[bullets.size() - 1].setFillColor(sf::Color::Magenta);
                    bullets[bullets.size() - 2].setFillColor(sf::Color::Magenta);
                }

                fireClock.restart();
            }

            //stars
            for (auto& star : stars)
            {
                star.move({ 0, 2 });
                if (star.getPosition().y > 600)
                    star.setPosition({ (float)(std::rand() % 800), 0 });
            }

            //bullets
            for (size_t i = 0; i < bullets.size(); i++)
            {
                bullets[i].move({ 0.f, -10 });
                if (bullets[i].getPosition().y < 0)
                {
                    bullets.erase(bullets.begin() + i);
                    i--;
                }
            }

            for (auto& e : enemies)
            {
                e.sprite.move({ 0.f, e.speed });

                if (e.sprite.getPosition().y > 600)
                {
                    e.sprite.setPosition({
                        static_cast<float>(std::rand() % 760),
                        -150.f
                        });
                }
            }

            for (size_t b = 0; b < bullets.size(); b++)
            {
                bool bulletDestroyed = false;

                for (auto& e : enemies)
                {
                    if (bullets[b].getGlobalBounds().findIntersection(e.sprite.getGlobalBounds()))
                    {

                        explosions.push_back({ sf::CircleShape(5.f), 1.f });
                        explosions.back().shape.setFillColor(sf::Color::Yellow);
                        explosions.back().shape.setPosition(e.sprite.getPosition());

                        e.sprite.setPosition({
                            static_cast<float>(std::rand() % 760),
                            -200.f
                            });

                        score += 10;
                        bulletDestroyed = true;
                        break;
                    }
                }

                if (bulletDestroyed)
                {
                    bullets.erase(bullets.begin() + b);
                    b--;
                }
            }

            if (hit)
            {
                for (auto& e : enemies)
                {
                    if (player.getGlobalBounds().findIntersection(e.sprite.getGlobalBounds()))
                    {
                        lives--;
                        player.setPosition({ 380.f, 520.f });
                        hit = false;
                        hitClock.restart();
                        break;
                    }
                }
            }

            if (!hit && hitClock.getElapsedTime().asSeconds() > 2.f)
            {
                hit = true;
            }
            if (lives <= 0)
            {
                gameOver = true;
            }

            for (size_t i = 0; i < explosions.size(); i++)
            {
                explosions[i].life -= 0.2f;
                explosions[i].shape.setRadius(explosions[i].shape.getRadius() + 2);
                if (explosions[i].life <= 0)
                {
                    explosions.erase(explosions.begin() + i);
                    i--;
                }
            }
        }


        window.clear(sf::Color::Black);

        for (auto& s : stars) window.draw(s);

        // lives
        for (int i = 0; i < lives; i++)
        {
            sf::CircleShape heart(6);
            heart.setFillColor(sf::Color::Red);
            heart.setPosition({ 10.f + i * 20.f, 10.f });
            window.draw(heart);
        }

        scoreText.setString("Score: " + std::to_string(score));
        window.draw(scoreText);

        for (auto& e : enemies) window.draw(e.sprite);
        for (auto& b : bullets) window.draw(b);
        for (auto& ex : explosions) window.draw(ex.shape);

        window.draw(player);

        if (gameOver)
        {
            finalScoreText.setString("Final Score: " + std::to_string(score));
            window.draw(gameOverText);
            window.draw(restartText);
            window.draw(finalScoreText);
        }

        window.display();
    }

    return 0;
}