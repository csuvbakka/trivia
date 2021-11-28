#include <iostream>
#include <list>
#include <string_view>
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
  void movePlayer(int n_steps);
  bool wasCorrectlyAnswered();
  void makeNextPlayerTheCurrent();
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

  std::vector<Player> players;

  std::list<std::string> popQuestions;
  std::list<std::string> scienceQuestions;
  std::list<std::string> sportsQuestions;
  std::list<std::string> rockQuestions;

  unsigned int currentPlayer;
  bool isGettingOutOfPenaltyBox;
};

#endif /* GAME_H_ */
