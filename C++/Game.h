#include <iostream>
#include <list>
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

class Game {
 public:
  using QuestionPool = std::unordered_map<Category, std::list<std::string>>;

  static Game Create(std::vector<std::string> playerNames, QuestionPool questionPool);
  bool isPlayable();

  void roll(int roll);
  bool wasCorrectlyAnswered();
  bool wrongAnswer();

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

  std::string nextQuestion(Category category);
  Category currentCategory();
  void updateCurrentPlayer();

  std::vector<Player> players_;
  QuestionPool questionPool_;

  std::vector<Player>::iterator currentPlayer_;
  bool isGettingOutOfPenaltyBox_;
};

#endif /* GAME_H_ */
