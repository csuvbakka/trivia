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
  using QuestionPool = std::unordered_map<Category, std::list<std::string>>;

  static Game Create(std::vector<std::string> playerNames, QuestionPool questionPool);
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

  Game(std::vector<Player> players, QuestionPool questionPool);

  std::optional<int> movePlayer(int roll);
  Question readQuestion();
  Answer askQuestion(Question question);
  void evaluateAnswer(Answer answer);

  std::string nextQuestion(Category category);
  void updateCurrentPlayer();

  bool didPlayerWin(const Player& player) const;

  std::vector<Player> players_;
  QuestionPool questionPool_;

  std::vector<Player>::iterator currentPlayer_;
  bool isGettingOutOfPenaltyBox_;
};

#endif /* GAME_H_ */
