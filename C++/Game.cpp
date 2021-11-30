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
    : players_(std::move(players)), questionPool_(std::move(questionPool)), currentPlayerId_(0)
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

void TriviaGame::run()
{
  currentPlayerId_ = -1;
  while (true) {
    updateCurrentPlayer();
    if (const auto newLocation = movePlayer()) {
      auto question = readQuestion(*newLocation);
      askQuestion(std::move(question));
    }

    evaluateAnswer(Answer{});
    if (didPlayerWin(currentPlayerId_))
      break;
  }
}

bool TriviaGame::isPlayable()
{
  return (players_.size() >= 2);
}

std::optional<int> TriviaGame::movePlayer()
{
  auto& currentPlayer = players_[currentPlayerId_];
  const int roll      = rand() % 5 + 1;
  std::cout << currentPlayer.name << " is the current player\n";
  std::cout << "They have rolled a " << roll << "\n";

  if (currentPlayer.state.inPenaltyBox) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox_ = true;
      std::cout << currentPlayer.name << " is getting out of the penalty box\n";
    } else {
      isGettingOutOfPenaltyBox_ = false;
      std::cout << currentPlayer.name << " is not getting out of the penalty box\n";
      return std::nullopt;
    }
  }

  currentPlayer.state.field = (currentPlayer.state.field + roll) % 12;
  std::cout << currentPlayer.name << "'s new location is " << currentPlayer.state.field << "\n";
  return currentPlayer.state.field;
}

Question TriviaGame::readQuestion(int location)
{
  const auto category = categoryForField(location);
  const auto question = nextQuestion(category);

  return {question, category};
}

Answer TriviaGame::askQuestion(Question question)
{
  std::cout << "The category is " << ToStringView(question.category) << "\n";
  std::cout << question.text << "\n";
  return {};
}

std::string TriviaGame::nextQuestion(Category category)
{
  auto& questionGroup = questionPool_[category];
  auto question       = questionGroup.front();
  questionGroup.pop_front();
  return question;
}

void TriviaGame::updateCurrentPlayer()
{
  ++currentPlayerId_;
  if (currentPlayerId_ == static_cast<int>(players_.size()))
    currentPlayerId_ = 0;
}

bool TriviaGame::didPlayerWin(int playerId) const
{
  return players_[playerId].state.coins == 6;
}

void TriviaGame::evaluateAnswer(Answer /*answer*/)
{
  auto& currentPlayer  = players_[currentPlayerId_];
  const bool isCorrect = rand() % 9 != 7;
  if (!isCorrect) {
    std::cout << "Question was incorrectly answered\n";
    std::cout << currentPlayer.name + " was sent to the penalty box\n";
    currentPlayer.state.inPenaltyBox = true;
    return;
  } else {
    if (currentPlayer.state.inPenaltyBox) {
      if (isGettingOutOfPenaltyBox_) {
        std::cout << "Answer was correct!!!!\n";
      } else {
        return;
      }
    } else {
      std::cout << "Answer was corrent!!!!\n";
    }
    currentPlayer.state.coins++;
    std::cout << currentPlayer.name << " now has " << currentPlayer.state.coins << " Gold Coins.\n";
  }
}
