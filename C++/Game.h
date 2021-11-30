#include <iostream>
#include <list>
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

class Game {
 public:
  virtual ~Game()                               = default;
  virtual std::optional<int> movePlayer()       = 0;
  virtual Question readQuestion(int location)   = 0;
  virtual Answer askQuestion(Question question) = 0;
  virtual void evaluateAnswer(Answer answer)    = 0;
  virtual bool didPlayerWin(int playerId) const = 0;
};

class TriviaGame : public Game {
 public:
  using QuestionPool = std::unordered_map<Category, std::list<std::string>>;

  static TriviaGame Create(std::vector<std::string> playerNames, QuestionPool questionPool);
  bool isPlayable();

  void run();

 private:
  struct PlayerState {
    PlayerState(int field, int coins, bool inPenaltyBox)
        : field(field), coins(coins), inPenaltyBox(inPenaltyBox)
    {
    }
    int field;
    int coins;
    bool inPenaltyBox;
  };

  struct Player {
    Player(std::string name, PlayerState state) : name(std::move(name)), state(state) {}
    std::string name;
    PlayerState state;
  };

  TriviaGame(std::vector<Player> players, QuestionPool questionPool);

  virtual std::optional<int> movePlayer() override;
  virtual Question readQuestion(int location) override;
  virtual Answer askQuestion(Question question) override;
  virtual void evaluateAnswer(Answer answer) override;

  std::string nextQuestion(Category category);
  void updateCurrentPlayer();

  virtual bool didPlayerWin(int playerId) const override;

  std::vector<Player> players_;
  QuestionPool questionPool_;

  int currentPlayerId_;
  bool isGettingOutOfPenaltyBox_;
};

#endif /* GAME_H_ */
