#include <iostream>
#include <list>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifndef GAME_H_
#define GAME_H_

enum class Category { Pop, Science, Sports, Rock };
constexpr std::string_view ToStringView(Category category);

class Game {
 public:
  Game();
  bool isPlayable();
  bool add(std::string playerName);

  void roll(int roll);
  bool wasCorrectlyAnswered();
  bool wrongAnswer();

 private:
  struct PlayerState {
    PlayerState(int place, int purse, bool inPenaltyBox)
        : place(place), purse(purse), inPenaltyBox(inPenaltyBox)
    {
    }
    int place;
    int purse;
    bool inPenaltyBox;
  };

  struct Player {
    Player(std::string name, PlayerState state) : name(std::move(name)), state(state) {}
    std::string name;
    PlayerState state;
  };

  bool didPlayerWin();
  void askQuestion();
  Category currentCategory();
  void movePlayer(int n_steps);
  void makeNextPlayerTheCurrent();

  std::vector<Player> players;
  std::unordered_map<Category, std::list<std::string>> questions;

  unsigned int currentPlayer;
  bool isGettingOutOfPenaltyBox;
};

#endif /* GAME_H_ */
