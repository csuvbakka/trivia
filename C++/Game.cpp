#include "Game.h"

Game::Game(int nPlayers) : nPlayers_(nPlayers) {}

void Game::run()
{
  int currentPlayerId = -1;
  while (true) {
    currentPlayerId = getNextPlayerId(currentPlayerId);
    auto turn       = newTurn(currentPlayerId);
    if (const auto newLocation = turn->movePlayer(turn->rollDice())) {
      auto question = turn->readQuestion(*newLocation);
      if (!question.has_value())
        continue;
      auto answer = turn->askQuestion(std::move(*question));
      if (turn->isAnswerCorrect(answer))
        turn->onCorrectAnswer();
      else
        turn->onIncorrectAnswer();

      if (didPlayerWin(currentPlayerId))
        break;
    }
  }
}

int Game::getNextPlayerId(int currentPlayerId) const
{
  return (currentPlayerId + 1) % nPlayers_;
}
