#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifndef GAME_H_
#define GAME_H_

enum class Category { Pop, Science, Sports, Rock };
constexpr std::string_view ToStringView(Category category)
{
  switch (category) {
    case Category::Pop: return "Pop";
    case Category::Science: return "Science";
    case Category::Sports: return "Sports";
    case Category::Rock: return "Rock";
  }
  throw std::runtime_error("Unknown category.");
}

struct Question {
  std::string text;
  Category category;
};
struct Answer {
};

struct Player {
  struct State {
    State(int field, int coins, bool inPenaltyBox)
        : field(field), coins(coins), inPenaltyBox(inPenaltyBox)
    {
    }
    int field;
    int coins;
    bool inPenaltyBox;
  };

  Player(std::string name, State state) : name(std::move(name)), state(state) {}
  std::string name;
  State state;
};
using QuestionPool = std::unordered_map<Category, std::list<std::string>>;

class GameTurn {
 public:
  virtual ~GameTurn() = default;

  virtual int rollDice() const                    = 0;
  virtual std::optional<int> movePlayer(int roll) = 0;
  virtual Question readQuestion(int location)     = 0;
  virtual Answer askQuestion(Question)            = 0;
  virtual bool isAnswerCorrect(Answer)            = 0;
  virtual void onCorrectAnswer()                  = 0;
  virtual void onIncorrectAnswer()                = 0;
};

class Game {
 public:
  Game(int nPlayers);
  virtual ~Game() = default;

  void run();

 private:
  int getNextPlayerId(int currentPlayerId) const;

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) = 0;
  virtual bool didPlayerWin(int playerId) const           = 0;

  int nPlayers_;
};

class TriviaGameTurn : public GameTurn {
 public:
  TriviaGameTurn(Player& player, QuestionPool& questionPool, std::ostream& logger);

  virtual int rollDice() const override;
  virtual std::optional<int> movePlayer(int roll) override;
  virtual Question readQuestion(int location) override;
  virtual Answer askQuestion(Question question) override;
  virtual bool isAnswerCorrect(Answer answer) override;
  virtual void onCorrectAnswer() override;
  virtual void onIncorrectAnswer() override;

 private:
  std::string nextQuestion(Category category);

  Player& player_;
  QuestionPool& questionPool_;
  std::ostream& logger_;
};

class TriviaGame : public Game {
 public:
  static TriviaGame Create(std::vector<std::string> playerNames,
                           QuestionPool questionPool,
                           std::ostream& logger);
  bool isPlayable();

 private:
  TriviaGame(std::vector<Player> players, QuestionPool questionPool, std::ostream& logger);

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) override;
  virtual bool didPlayerWin(int playerId) const override;

  std::vector<Player> players_;
  QuestionPool questionPool_;
  std::ostream& logger_;
};

#endif /* GAME_H_ */
