#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

namespace {

Category categoryForField(int field)
{
  switch (field % 4) {
    case 0: return Category::Pop;
    case 1: return Category::Science;
    case 2: return Category::Sports;
    default: return Category::Rock;
  }
}
}  // namespace

TriviaGame::TriviaGame(std::vector<Player> players, QuestionPool questionPool)
    : Game(players.size()), players_(std::move(players)), questionPool_(std::move(questionPool))
{
}

TriviaGame TriviaGame::Create(std::vector<std::string> playerNames, QuestionPool questionPool)
{
  std::vector<Player> players;
  players.reserve(playerNames.size());

  for (auto&& name : playerNames) {
    players.emplace_back(std::move(name), PlayerState{0, 0, false});
    std::cout << players.back().name << " was added\n";
    std::cout << "They are player number " << players.size() << "\n";
  }

  return TriviaGame(std::move(players), std::move(questionPool));
}

bool TriviaGame::isPlayable()
{
  return (players_.size() >= 2);
}

std::unique_ptr<GameTurn> TriviaGame::newTurn(int playerId)
{
  return std::make_unique<TriviaGameTurn>(players_[playerId], questionPool_);
}

bool TriviaGame::didPlayerWin(int playerId) const
{
  return players_[playerId].state.coins == 6;
}

Game::Game(int nPlayers) : nPlayers_(nPlayers) {}

void Game::run()
{
  int currentPlayerId = -1;
  while (true) {
    currentPlayerId = getNextPlayerId(currentPlayerId);
    auto turn       = newTurn(currentPlayerId);
    if (const auto newLocation = turn->movePlayer()) {
      auto question = turn->readQuestion(*newLocation);
      turn->askQuestion(std::move(question));
    }

    turn->evaluateAnswer(Answer{});
    if (didPlayerWin(currentPlayerId))
      break;
  }
}

int Game::getNextPlayerId(int currentPlayerId) const
{
  return (currentPlayerId + 1) % nPlayers_;
}

TriviaGameTurn::TriviaGameTurn(Player& player, QuestionPool& questionPool)
    : player_(player), questionPool_(questionPool)
{
}

std::optional<int> TriviaGameTurn::movePlayer()
{
  const int roll = rand() % 5 + 1;
  std::cout << player_.name << " is the current player\n";
  std::cout << "They have rolled a " << roll << "\n";

  if (player_.state.inPenaltyBox) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox_ = true;
      std::cout << player_.name << " is getting out of the penalty box\n";
    } else {
      isGettingOutOfPenaltyBox_ = false;
      std::cout << player_.name << " is not getting out of the penalty box\n";
      return std::nullopt;
    }
  }

  player_.state.field = (player_.state.field + roll) % 12;
  std::cout << player_.name << "'s new location is " << player_.state.field << "\n";
  return player_.state.field;
}

Question TriviaGameTurn::readQuestion(int location)
{
  const auto category = categoryForField(location);
  const auto question = nextQuestion(category);

  return {question, category};
}

Answer TriviaGameTurn::askQuestion(Question question)
{
  std::cout << "The category is " << ToStringView(question.category) << "\n";
  std::cout << question.text << "\n";
  return {};
}

void TriviaGameTurn::evaluateAnswer(Answer /*answer*/)
{
  const bool isCorrect = rand() % 9 != 7;
  if (!isCorrect) {
    std::cout << "Question was incorrectly answered\n";
    std::cout << player_.name + " was sent to the penalty box\n";
    player_.state.inPenaltyBox = true;
    return;
  } else {
    if (player_.state.inPenaltyBox) {
      if (isGettingOutOfPenaltyBox_) {
        std::cout << "Answer was correct!!!!\n";
      } else {
        return;
      }
    } else {
      std::cout << "Answer was corrent!!!!\n";
    }
    player_.state.coins++;
    std::cout << player_.name << " now has " << player_.state.coins << " Gold Coins.\n";
  }
}

std::string TriviaGameTurn::nextQuestion(Category category)
{
  auto& questionGroup = questionPool_[category];
  auto question       = questionGroup.front();
  questionGroup.pop_front();
  return question;
}
