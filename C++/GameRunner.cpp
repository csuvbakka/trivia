#include <stdlib.h>
#include "Game.h"

#include <string>

TriviaGame::QuestionPool generateQuestions()
{
  TriviaGame::QuestionPool questionPool;
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
  auto aGame = TriviaGame::Create({"Chet", "Pat", "Sue"}, generateQuestions());
  aGame.run();
}
