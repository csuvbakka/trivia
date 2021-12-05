#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

#include "TriviaGame.h"

#include <functional>
#include <iostream>
#include <sstream>

namespace {
using Catch::Matchers::EndsWith;
using Catch::Matchers::StartsWith;

TriviaQuestionPool createTestQuestions()
{
  return {
      {Category::Pop, {"Pop 1", "Pop 2", "Pop 3"}},
      {Category::Rock, {"Rock 1", "Rock 2", "Rock 3"}},
      {Category::Science, {"Science 1", "Science 2", "Science 3"}},
      {Category::Sports, {"Sports 1", "Sports 2", "Sports 3"}},
  };
}

struct DevNullLogger : private std::streambuf, public std::ostream {
  DevNullLogger() : std::ostream(this) {}

 private:
  virtual int overflow(int) override { return 0; }
};

DevNullLogger devNull;

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
  virtual std::optional<Question> readQuestion(int location) override
  {
    return readQuestion_(location);
  }
  virtual Answer askQuestion(Question question) override { return askQuestion_(question); }
  virtual bool isAnswerCorrect(Answer answer) override { return isAnswerCorrect_(answer); }
  virtual void onCorrectAnswer() override { return onCorrectAnswer_(); }
  virtual void onIncorrectAnswer() override { return onIncorrectAnswer_(); }

  std::function<int()> rollDice_;
  std::function<std::optional<int>(int)> movePlayer_;
  std::function<std::optional<Question>(int)> readQuestion_;
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

TEST_CASE("Players roll the dice then move that many places.", "[Game]")
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

TEST_CASE("Players turn is over if they cannot move.", "[Game]")
{
  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int playerId) {
    if (playerId == 1)
      return std::make_unique<FakeGameTurn>();

    auto turn          = std::make_unique<FakeGameTurn>();
    turn->movePlayer_  = [](int) { return std::nullopt; };
    turn->askQuestion_ = [&](Question) -> Answer {
      throw "Unexpected function call askQuestion().";
    };
    turn->isAnswerCorrect_ = [&](Answer) -> bool {
      throw "Unexpected function call isAnswerCorrect().";
    };
    turn->readQuestion_ = [&](int) -> std::optional<Question> {
      throw "Unexpected function call readQuestion().";
    };
    return turn;
  };
  game.run();
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

TEST_CASE("Players turn is over if there are no more questions.", "[Game]")
{
  const int nPlayers = 3;
  const int nTurns   = 5;
  auto game          = FakeGame(nPlayers, nTurns);
  game.newTurn_      = [&](int playerId) {
    if (playerId == 1)
      return std::make_unique<FakeGameTurn>();

    auto turn           = std::make_unique<FakeGameTurn>();
    turn->readQuestion_ = [&](int) { return std::nullopt; };
    turn->askQuestion_  = [&](Question) -> Answer {
      throw "Unexpected function call askQuestion().";
    };
    turn->isAnswerCorrect_ = [&](Answer) -> bool {
      throw "Unexpected function call isAnswerCorrect().";
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

TEST_CASE("Question categories are assigned in cyclical order.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = TriviaPlayer{"test player", {0, 0, false}};

  auto turn = TriviaGameTurn(player, questionPool, devNull);
  REQUIRE(turn.readQuestion(0)->category == Category::Pop);
  REQUIRE(turn.readQuestion(1)->category == Category::Science);
  REQUIRE(turn.readQuestion(2)->category == Category::Sports);
  REQUIRE(turn.readQuestion(3)->category == Category::Rock);
  REQUIRE(turn.readQuestion(4)->category == Category::Pop);
  REQUIRE(turn.readQuestion(5)->category == Category::Science);
  REQUIRE(turn.readQuestion(6)->category == Category::Sports);
  REQUIRE(turn.readQuestion(7)->category == Category::Rock);
  REQUIRE(turn.readQuestion(8)->category == Category::Pop);
  REQUIRE(turn.readQuestion(9)->category == Category::Science);
  REQUIRE(turn.readQuestion(10)->category == Category::Sports);
  REQUIRE(turn.readQuestion(11)->category == Category::Rock);
}

TEST_CASE("Pops questions from the pool with category corresponding to the location.",
          "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = TriviaPlayer{"test player", {0, 0, false}};

  auto turn = TriviaGameTurn(player, questionPool, devNull);
  SECTION("Category Pop")
  {
    turn.readQuestion(0);
    REQUIRE(questionPool[Category::Pop].size() == 2);
  }
  SECTION("Category Science")
  {
    turn.readQuestion(1);
    REQUIRE(questionPool[Category::Science].size() == 2);
  }
  SECTION("Category Sports")
  {
    turn.readQuestion(2);
    REQUIRE(questionPool[Category::Sports].size() == 2);
  }
  SECTION("Category Rock")
  {
    turn.readQuestion(3);
    REQUIRE(questionPool[Category::Rock].size() == 2);
  }
}

TEST_CASE("Questions are consumed in the order they were added.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = TriviaPlayer{"test player", {0, 0, false}};

  auto turn = TriviaGameTurn(player, questionPool, devNull);
  REQUIRE(turn.readQuestion(0)->text == "Pop 1");
  REQUIRE(turn.readQuestion(0)->text == "Pop 2");
}

TEST_CASE("The question is logged.", "[TriviaGameTurn]")
{
  auto player       = TriviaPlayer{"test player", {0, 0, false}};
  auto questionPool = createTestQuestions();

  std::ostringstream logger;
  auto turn = TriviaGameTurn(player, questionPool, logger);
  turn.askQuestion(Question{"test question", Category::Rock});

  REQUIRE(logger.str() == "The category is Rock\ntest question\n");
}

TEST_CASE("Dice roll is logged.", "[TriviaGameTurn]")
{
  auto player       = TriviaPlayer{"test player", {0, 0, false}};
  auto questionPool = createTestQuestions();

  std::ostringstream logger;
  auto turn = TriviaGameTurn(player, questionPool, logger);
  turn.rollDice();

  REQUIRE_THAT(logger.str(),
               StartsWith(player.name + " is the current player\nThey have rolled a "));
}

TEST_CASE("Player movement.", "[TriviaGameTurn]")
{
  auto player       = TriviaPlayer{"test player", {0, 0, false}};
  auto questionPool = createTestQuestions();

  SECTION("Player can move if they are not in the penalty box.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool, devNull);
    player.state.inPenaltyBox = false;
    player.state.field        = 0;

    auto newLocation = turn.movePlayer(3);
    CHECK(newLocation.value() == 3);
    REQUIRE(player.state.field == 3);
  }
  SECTION("Player can move if they are in the penalty box and rolled odd.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool, devNull);
    player.state.inPenaltyBox = true;
    player.state.field        = 0;

    auto newLocation = turn.movePlayer(3);
    CHECK(newLocation.value() == 3);
    REQUIRE(player.state.field == 3);
  }
  SECTION("Player cannot move if they are in the penalty box and rolled even.")
  {
    auto turn                 = TriviaGameTurn(player, questionPool, devNull);
    player.state.inPenaltyBox = true;
    player.state.field        = 1;

    auto newLocation = turn.movePlayer(2);
    CHECK(!newLocation.has_value());
    REQUIRE(player.state.field == 1);
  }
  SECTION("The board is circular and has 12 fields.")
  {
    auto turn          = TriviaGameTurn(player, questionPool, devNull);
    player.state.field = 10;
    turn.movePlayer(7);
    REQUIRE(player.state.field == 5);
    turn.movePlayer(7);
    REQUIRE(player.state.field == 0);
    turn.movePlayer(13);
    REQUIRE(player.state.field == 1);
  }
  SECTION("Player's new location is logged if they are not in the penalty box.")
  {
    std::ostringstream logger;
    auto turn                 = TriviaGameTurn(player, questionPool, logger);
    player.state.inPenaltyBox = false;
    player.state.field        = 1;

    turn.movePlayer(2);
    REQUIRE(logger.str() == player.name + "'s new location is 3\n");
  }
  SECTION("Player's new location is logged if they are in the penalty box and rolled odd.")
  {
    std::ostringstream logger;
    auto turn                 = TriviaGameTurn(player, questionPool, logger);
    player.state.inPenaltyBox = true;
    player.state.field        = 1;

    turn.movePlayer(3);
    const std::string expectedMsg = player.name + " is getting out of the penalty box\n"
                                    + player.name + "'s new location is 4\n";
    REQUIRE(logger.str() == expectedMsg);
  }
  SECTION("Player's new location is not logged if they are in the penalty box and rolled even.")
  {
    std::ostringstream logger;
    auto turn                 = TriviaGameTurn(player, questionPool, logger);
    player.state.inPenaltyBox = true;
    player.state.field        = 1;

    turn.movePlayer(2);
    const std::string expectedMsg = player.name + " is not getting out of the penalty box\n";
    REQUIRE(logger.str() == expectedMsg);
  }
}

TEST_CASE("If a player answers correctly.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = TriviaPlayer{"test player", {0, 0, false}};

  SECTION("They get a gold coin.")
  {
    player.state.coins = 0;
    auto turn          = TriviaGameTurn(player, questionPool, devNull);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 1);
    turn.onCorrectAnswer();
    REQUIRE(player.state.coins == 2);
  }
  SECTION("The number of coins is logged.")
  {
    std::ostringstream logger;
    player.state.coins = 0;
    auto turn          = TriviaGameTurn(player, questionPool, logger);
    turn.onCorrectAnswer();
    REQUIRE(logger.str() == "Answer was correct!!!!\n" + player.name + " now has 1 Gold Coins.\n");
  }
}

TEST_CASE("If a player answers incorrectly.", "[TriviaGameTurn]")
{
  auto questionPool = createTestQuestions();
  auto player       = TriviaPlayer{"test player", {0, 0, false}};

  SECTION("They are sent to the penalty box.")
  {
    player.state.inPenaltyBox = false;
    auto turn                 = TriviaGameTurn(player, questionPool, devNull);
    turn.onIncorrectAnswer();
    REQUIRE(player.state.inPenaltyBox);
  }
  SECTION("They get no gold coins.")
  {
    player.state.coins = 0;
    auto turn          = TriviaGameTurn(player, questionPool, devNull);
    turn.onIncorrectAnswer();
    REQUIRE(player.state.coins == 0);
  }
  SECTION("It is added to the log.")
  {
    std::ostringstream logger;
    auto turn = TriviaGameTurn(player, questionPool, logger);
    turn.onIncorrectAnswer();
    REQUIRE(logger.str()
            == "Question was incorrectly answered\n" + player.name
                   + " was sent to the penalty box\n");
  }
}

TEST_CASE("A game requires at least 2 players.", "[TriviaGame]")
{
  auto game = TriviaGame::Create({"a single player"}, createTestQuestions(), devNull);
  REQUIRE(!game.has_value());
}

TEST_CASE("Player info is logged when the game is created.", "[TriviaGame]")
{
  std::ostringstream logger;
  auto game = TriviaGame::Create({"A", "B"}, createTestQuestions(), logger);
  const std::string expectedMsg =
      "A was added\nThey are player number 1\nB was added\nThey are player number 2\n";
  REQUIRE(logger.str() == expectedMsg);
}
}  // namespace
