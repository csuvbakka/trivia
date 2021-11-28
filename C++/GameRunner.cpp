#include <stdlib.h>
#include "Game.h"

static bool notAWinner;

int main()
{
  srand(time(NULL));
  auto aGame = Game::Create({"Chet", "Pat", "Sue"});

  do {
    aGame.roll(rand() % 5 + 1);

    if (rand() % 9 == 7) {
      notAWinner = aGame.wrongAnswer();
    } else {
      notAWinner = aGame.wasCorrectlyAnswered();
    }
  } while (notAWinner);
}
