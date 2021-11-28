#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

Game::Game(std::vector<Player> players, QuestionPool questionPool)
    : players_(std::move(players))
    , questionPool_(std::move(questionPool))
    , currentPlayer_(players_.begin())
{
}

Game Game::Create(std::vector<std::string> playerNames, QuestionPool questionPool)
{
  std::vector<Player> players;
  players.reserve(playerNames.size());

  for (auto&& name : playerNames) {
    players.emplace_back(std::move(name), PlayerState{0, 0, false});
    std::cout << players.back().name << " was added\n";
    std::cout << "They are player number " << players.size() << "\n";
  }

  return Game(std::move(players), std::move(questionPool));
}

bool Game::isPlayable()
{
  return (players_.size() >= 2);
}

void Game::roll(int roll)
{
  std::cout << currentPlayer_->name << " is the current player\n";
  std::cout << "They have rolled a " << roll << "\n";

  if (currentPlayer_->state.inPenaltyBox) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox_ = true;
      std::cout << currentPlayer_->name << " is getting out of the penalty box\n";
    } else {
      isGettingOutOfPenaltyBox_ = false;
      std::cout << currentPlayer_->name << " is not getting out of the penalty box\n";
      return;
    }
  }

  movePlayer(roll);
  std::cout << currentPlayer_->name << "'s new location is " << currentPlayer_->state.place << "\n";
  std::cout << "The category is " << ToStringView(currentCategory()) << "\n";
  askQuestion(currentCategory());
}

void Game::movePlayer(int n_steps)
{
  currentPlayer_->state.place = (currentPlayer_->state.place + n_steps) % 12;
}

void Game::askQuestion(Category category)
{
  auto& questionGroup = questionPool_[category];
  std::cout << questionGroup.front() << "\n";
  questionGroup.pop_front();
}

Category Game::currentCategory()
{
  switch (currentPlayer_->state.place % 4) {
    case 0: return Category::Pop;
    case 1: return Category::Science;
    case 2: return Category::Sports;
    default: return Category::Rock;
  }
}

bool Game::wasCorrectlyAnswered()
{
  if (currentPlayer_->state.inPenaltyBox) {
    if (isGettingOutOfPenaltyBox_) {
      std::cout << "Answer was correct!!!!\n";
    } else {
      makeNextPlayerTheCurrent();
      return true;
    }
  } else {
    std::cout << "Answer was corrent!!!!\n";
  }
  currentPlayer_->state.purse++;
  std::cout << currentPlayer_->name << " now has " << currentPlayer_->state.purse
            << " Gold Coins.\n";

  const bool didPlayerWin = currentPlayer_->state.purse == 6;
  makeNextPlayerTheCurrent();

  return !didPlayerWin;
}

void Game::makeNextPlayerTheCurrent()
{
  ++currentPlayer_;
  if (currentPlayer_ == players_.end())
    currentPlayer_ = players_.begin();
}

bool Game::wrongAnswer()
{
  std::cout << "Question was incorrectly answered\n";
  std::cout << currentPlayer_->name + " was sent to the penalty box\n";
  currentPlayer_->state.inPenaltyBox = true;

  makeNextPlayerTheCurrent();
  return true;
}
