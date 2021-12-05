#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "Game.h"

#include <functional>
QuestionPool createTestQuestions()
{
  return {
      {Category::Pop, {"Pop 1", "Pop 2", "Pop 3"}},
      {Category::Rock, {"Rock 1", "Rock 2", "Rock 3"}},
      {Category::Science, {"Science 1", "Science 2", "Science 3"}},
      {Category::Sports, {"Sports 1", "Sports 2", "Sports 3"}},
  };
}

struct FakeGameTurn : public GameTurn {
  FakeGameTurn()
      : rollDice_([]() { return 1; })
      , movePlayer_([](int roll) { return roll; })
      , readQuestion_([](int) {
        return Question{"test question", Category::Pop};
      })
      , askQuestion_([](Question) { return Answer{}; })
      , isAnswerCorrect_([](Answer) { return false; })
      , onCorrectAnswer_([] {})
      , onIncorrectAnswer_([] {})
  {
  }
  virtual int rollDice() const override { return rollDice_(); }
  virtual std::optional<int> movePlayer(int roll) override { return movePlayer_(roll); }
  virtual Question readQuestion(int location) override { return readQuestion_(location); }
  virtual Answer askQuestion(Question question) override { return askQuestion_(question); }
  virtual bool isAnswerCorrect(Answer answer) override { return isAnswerCorrect_(answer); }
  virtual void onCorrectAnswer() override { return onCorrectAnswer_(); }
  virtual void onIncorrectAnswer() override { return onIncorrectAnswer_(); }

  std::function<int()> rollDice_;
  std::function<std::optional<int>(int)> movePlayer_;
  std::function<Question(int)> readQuestion_;
  std::function<Answer(Question)> askQuestion_;
  std::function<bool(Answer)> isAnswerCorrect_;
  std::function<void()> onCorrectAnswer_;
  std::function<void()> onIncorrectAnswer_;
};

struct FakeGame : public Game {
  FakeGame(int nPlayers, int nTurns)
      : Game(nPlayers)
      , newTurn_([](int /*playerId*/) { return std::make_unique<FakeGameTurn>(); })
      , currentTurn_(0)
      , nTurns_(nTurns)
  {
  }

  int currentTurn() const { return currentTurn_; }

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) override
  {
    ++currentTurn_;
    return newTurn_(playerId);
  }
  virtual bool didPlayerWin(int) const override { return currentTurn_ == nTurns_; }

  std::function<std::unique_ptr<GameTurn>(int)> newTurn_;

 private:
  int currentTurn_;
  int nTurns_;
};

TEST_CASE("Players take turns in cyclical order.", "[Game]")
{
  std::vector<int> playerIds;

  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int playerId) {
    playerIds.push_back(playerId);
    return std::make_unique<FakeGameTurn>();
  };
  game.run();
  REQUIRE(playerIds == std::vector<int>{0, 1, 2, 0, 1});
}

TEST_CASE("Players roll the dice then move.", "[Game]")
{
  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int playerId) {
    auto turn         = std::make_unique<FakeGameTurn>();
    turn->rollDice_   = [playerId] { return playerId; };
    turn->movePlayer_ = [playerId](int roll) {
      REQUIRE(roll == playerId);
      return 1;
    };
    return turn;
  };
  game.run();
}

TEST_CASE("Players move then get a question corresponding to their current location.", "[Game]")
{
  std::vector<int> locations;

  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int /*playerId*/) {
    auto turn           = std::make_unique<FakeGameTurn>();
    turn->movePlayer_   = [&game](int) { return game.currentTurn(); };
    turn->readQuestion_ = [&locations](int location) {
      locations.push_back(location);
      return Question{"test question", Category::Rock};
    };
    return turn;
  };
  game.run();
  REQUIRE(locations == std::vector<int>{1, 2, 3, 4, 5});
}

TEST_CASE("Players are asked questions.", "[Game]")
{
  std::vector<int> locations;

  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int /*playerId*/) {
    auto turn           = std::make_unique<FakeGameTurn>();
    turn->movePlayer_   = [&game](int) { return game.currentTurn(); };
    turn->readQuestion_ = [&locations](int location) {
      locations.push_back(location);
      return Question{"test question " + std::to_string(location), Category::Science};
    };
    turn->askQuestion_ = [&game](Question question) {
      CHECK(question.category == Category::Science);
      CHECK(question.text == "test question " + std::to_string(game.currentTurn()));
      return Answer{};
    };
    return turn;
  };
  game.run();
}

TEST_CASE("Answers are evaluated.", "[Game]")
{
  int nAnswers       = 0;
  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int /*playerId*/) {
    auto turn              = std::make_unique<FakeGameTurn>();
    turn->isAnswerCorrect_ = [&](Answer) {
      REQUIRE(++nAnswers == game.currentTurn());
      return false;
    };
    return turn;
  };
  game.run();
}

TEST_CASE("Game ends if the win condition is satisfied.", "[Game]")
{
  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.run();
  REQUIRE(game.currentTurn() == 5);
}

TEST_CASE("Pops questions from the pool with category corresponding to the location.",
          "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = Player{"test player", {0, 0, false}};

  auto turn = TriviaGameTurn(player, questionPool);
  SECTION("Category Pop")
  {
    REQUIRE(turn.readQuestion(0).category == Category::Pop);
    REQUIRE(questionPool[Category::Pop].size() == 2);
    REQUIRE(turn.readQuestion(4).category == Category::Pop);
    REQUIRE(questionPool[Category::Pop].size() == 1);
    REQUIRE(turn.readQuestion(8).category == Category::Pop);
    REQUIRE(questionPool[Category::Pop].size() == 0);
  }
  SECTION("Category Science")
  {
    REQUIRE(turn.readQuestion(1).category == Category::Science);
    REQUIRE(questionPool[Category::Science].size() == 2);
    REQUIRE(turn.readQuestion(5).category == Category::Science);
    REQUIRE(questionPool[Category::Science].size() == 1);
    REQUIRE(turn.readQuestion(9).category == Category::Science);
    REQUIRE(questionPool[Category::Science].size() == 0);
  }
  SECTION("Category Sports")
  {
    REQUIRE(turn.readQuestion(2).category == Category::Sports);
    REQUIRE(questionPool[Category::Sports].size() == 2);
    REQUIRE(turn.readQuestion(6).category == Category::Sports);
    REQUIRE(questionPool[Category::Sports].size() == 1);
    REQUIRE(turn.readQuestion(10).category == Category::Sports);
    REQUIRE(questionPool[Category::Sports].size() == 0);
  }
  SECTION("Category Rock")
  {
    REQUIRE(turn.readQuestion(3).category == Category::Rock);
    REQUIRE(questionPool[Category::Rock].size() == 2);
    REQUIRE(turn.readQuestion(7).category == Category::Rock);
    REQUIRE(questionPool[Category::Rock].size() == 1);
    REQUIRE(turn.readQuestion(11).category == Category::Rock);
    REQUIRE(questionPool[Category::Rock].size() == 0);
  }
}

TEST_CASE("Players move after rolling the dice.", "[TriviaGameTurn]")
{
  auto player       = Player{"test player", {0, 0, false}};
  auto questionPool = createTestQuestions();

  SECTION("Player can move if they are not in the penalty box.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool);
    player.state.inPenaltyBox = false;
    player.state.field        = 0;

    auto newLocation = turn.movePlayer(3);
    CHECK(newLocation.value() == 3);
    REQUIRE(player.state.field == 3);
  }
  SECTION("Player can move if they are in the penalty box, but rolled odd.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool);
    player.state.inPenaltyBox = true;
    player.state.field        = 0;

    auto newLocation = turn.movePlayer(3);
    CHECK(newLocation.value() == 3);
    REQUIRE(player.state.field == 3);
  }
  SECTION("Player cannot move if they are in the penalty box and rolled even.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool);
    player.state.inPenaltyBox = true;
    player.state.field        = 1;

    auto newLocation = turn.movePlayer(2);
    CHECK(!newLocation.has_value());
    REQUIRE(player.state.field == 1);
  }
}

TEST_CASE("If a player answers correctly.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = Player{"test player", {0, 0, false}};

  SECTION("They get a gold coin if they are not in the penalty box.")
  {
    player.state.inPenaltyBox = false;
    player.state.coins        = 0;
    auto turn                 = TriviaGameTurn(player, questionPool);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 1);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 2);
  }
  SECTION("They get a gold coin if they are in the penalty box but rolled odd.")
  {
    player.state.inPenaltyBox = true;
    player.state.coins        = 0;
    auto turn                 = TriviaGameTurn(player, questionPool);
    turn.movePlayer(3);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 1);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 2);
  }
  SECTION("They get no gold coins if they are in the penalty box and rolled even.")
  {
    player.state.inPenaltyBox = true;
    player.state.coins        = 0;
    auto turn                 = TriviaGameTurn(player, questionPool);
    turn.movePlayer(4);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 0);
  }
}

TEST_CASE("If a player answers incorrectly.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = Player{"test player", {0, 0, false}};

  SECTION("They are sent to the penalty box.")
  {
    player.state.inPenaltyBox = false;
    auto turn                 = TriviaGameTurn(player, questionPool);
    turn.onIncorrectAnswer();
    REQUIRE(player.state.inPenaltyBox);
  }
  SECTION("They get no gold coins.")
  {
    player.state.coins = 0;
    auto turn          = TriviaGameTurn(player, questionPool);
    turn.onIncorrectAnswer();
    REQUIRE(player.state.coins == 0);
  }
}
