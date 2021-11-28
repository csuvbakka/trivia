#include <iostream>
#include <list>
#include <vector>

#ifndef GAME_H_
#define GAME_H_

class Game {
 private:
  std::vector<std::string> players;

  int places[6];
  int purses[6];

  bool inPenaltyBox[6];

  std::list<std::string> popQuestions;
  std::list<std::string> scienceQuestions;
  std::list<std::string> sportsQuestions;
  std::list<std::string> rockQuestions;

  unsigned int currentPlayer;
  bool isGettingOutOfPenaltyBox;

 public:
  Game();
  bool isPlayable();
  bool add(std::string playerName);

  void roll(int roll);

 private:
  void askQuestion();
  std::string currentCategory();

 public:
  bool wasCorrectlyAnswered();
  bool wrongAnswer();

 private:
  bool didPlayerWin();
};

#endif /* GAME_H_ */
