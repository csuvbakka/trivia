#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Game.h"

#include <functional>

struct FakeGameTurn : public GameTurn {
  FakeGameTurn()
      : movePlayer_([]() { return 1; })
      , readQuestion_([](int) {
        return Question{"test question", Category::Pop};
      })
      , askQuestion_([](Question) { return Answer{}; })
      , evaluateAnswer_([](Answer) {})
  {
  }
  virtual std::optional<int> movePlayer() override { return movePlayer_(); }
  virtual Question readQuestion(int location) override { return readQuestion_(location); }
  virtual Answer askQuestion(Question question) override { return askQuestion_(question); }
  virtual void evaluateAnswer(Answer answer) override { return evaluateAnswer_(answer); }

  std::function<std::optional<int>()> movePlayer_;
  std::function<Question(int)> readQuestion_;
  std::function<Answer(Question)> askQuestion_;
  std::function<void(Answer)> evaluateAnswer_;
};

struct FakeGame : public Game {
  FakeGame(int nPlayers) : Game(nPlayers), didPlayerWin_([](int) { return false; }) {}

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) override { return newTurn_(playerId); }
  virtual bool didPlayerWin(int playerId) const override { return didPlayerWin_(playerId); }

  std::function<std::unique_ptr<GameTurn>(int)> newTurn_;
  std::function<bool(int)> didPlayerWin_;
};

TEST_CASE("Players take turns in cyclical order.", "[Game]")
{
  int turnCounter = 0;
  std::vector<int> playerIds;

  auto game     = FakeGame(3);
  game.newTurn_ = [&](int playerId) {
    turnCounter += 1;
    playerIds.push_back(playerId);
    return std::make_unique<FakeGameTurn>();
  };
  game.didPlayerWin_ = [&turnCounter](int) { return turnCounter == 5; };
  game.run();

  CHECK(turnCounter == 5);
  REQUIRE(playerIds == std::vector<int>{0, 1, 2, 0, 1});
}

TEST_CASE("Players move then get a question corresponding to their current location.", "[Game]")
{
  int turnCounter = 0;
  std::vector<int> locations;

  auto game     = FakeGame(3);
  game.newTurn_ = [&](int /*playerId*/) {
    turnCounter += 1;
    auto turn           = std::make_unique<FakeGameTurn>();
    turn->movePlayer_   = [&turnCounter]() { return turnCounter; };
    turn->readQuestion_ = [&locations](int location) {
      locations.push_back(location);
      return Question{"test question", Category::Rock};
    };
    return turn;
  };
  game.didPlayerWin_ = [&turnCounter](int) { return turnCounter == 5; };
  game.run();

  CHECK(turnCounter == 5);
  REQUIRE(locations == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("Players are asked questions.", "[Game]")
{
  int turnCounter = 0;
  std::vector<int> locations;

  auto game     = FakeGame(3);
  game.newTurn_ = [&](int /*playerId*/) {
    turnCounter += 1;
    auto turn           = std::make_unique<FakeGameTurn>();
    turn->movePlayer_   = [&turnCounter]() { return turnCounter; };
    turn->readQuestion_ = [&locations](int location) {
      locations.push_back(location);
      return Question{"test question " + std::to_string(location), Category::Science};
    };
    turn->askQuestion_ = [&turnCounter](Question question) {
      CHECK(question.category == Category::Science);
      CHECK(question.text == "test question " + std::to_string(turnCounter));
      return Answer{};
    };
    return turn;
  };
  game.didPlayerWin_ = [&turnCounter](int) { return turnCounter == 5; };
  game.run();
}

TEST_CASE("Answers are evaluated.", "[Game]")
{
  int turnCounter = 0;
  int nAnswers    = 0;

  auto game     = FakeGame(3);
  game.newTurn_ = [&](int /*playerId*/) {
    turnCounter += 1;
    auto turn             = std::make_unique<FakeGameTurn>();
    turn->evaluateAnswer_ = [&](Answer) { REQUIRE(++nAnswers == turnCounter); };
    return turn;
  };
  game.didPlayerWin_ = [&turnCounter](int) { return turnCounter == 5; };
  game.run();
}

TEST_CASE("Game ends if the win condition is satisfied.", "[Game]")
{
  int turnCounter = 0;

  auto game     = FakeGame(3);
  game.newTurn_ = [&](int /*playerId*/) {
    turnCounter += 1;
    return std::make_unique<FakeGameTurn>();
  };
  game.didPlayerWin_ = [&turnCounter](int) { return turnCounter == 5; };
  game.run();
  REQUIRE(turnCounter == 5);
}
