#include <stdlib.h>
#include "Game.h"

#include <string>

Game::QuestionPool generateQuestions()
{
  Game::QuestionPool questionPool;
  const auto generateQuestion = [&questionPool](Category category, int index) {
    questionPool[category].emplace_back(std::string(ToStringView(category)) + " Question "
                                        + std::to_string(index));
  };
  for (int i = 0; i < 50; i++) {
    generateQuestion(Category::Pop, i);
    generateQuestion(Category::Science, i);
    generateQuestion(Category::Sports, i);
    generateQuestion(Category::Rock, i);
  }
  return questionPool;
}

int main()
{
  srand(time(NULL));
  auto aGame      = Game::Create({"Chet", "Pat", "Sue"}, generateQuestions());
  bool notAWinner = true;
  do {
    aGame.rollWithNextPlayer(rand() % 5 + 1);

    if (rand() % 9 == 7) {
      notAWinner = aGame.wrongAnswer();
    } else {
      notAWinner = aGame.wasCorrectlyAnswered();
    }
  } while (notAWinner);
}
