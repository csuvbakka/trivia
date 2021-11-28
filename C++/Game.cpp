#include "Game.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

Game::Game(std::vector<Player> players, QuestionPool questionPool)
    : players_(std::move(players)), questionPool_(std::move(questionPool)), currentPlayer_(0)
{
}

Game Game::Create(std::vector<std::string> playerNames, QuestionPool questionPool)
{
  std::vector<Player> players;
  players.reserve(playerNames.size());

  for (auto&& name : playerNames) {
    players.emplace_back(std::move(name), PlayerState{0, 0, false});
    std::cout << players.back().name << " was added" << std::endl;
    std::cout << "They are player number " << players.size() << std::endl;
  }

  return Game(std::move(players), std::move(questionPool));
}

bool Game::isPlayable()
{
  return (players_.size() >= 2);
}

void Game::roll(int roll)
{
  std::cout << players_[currentPlayer_].name << " is the current player" << std::endl;
  std::cout << "They have rolled a " << roll << std::endl;

  if (players_[currentPlayer_].state.inPenaltyBox) {
    if (roll % 2 != 0) {
      isGettingOutOfPenaltyBox_ = true;
      std::cout << players_[currentPlayer_].name << " is getting out of the penalty box"
                << std::endl;
    } else {
      isGettingOutOfPenaltyBox_ = false;
      std::cout << players_[currentPlayer_].name << " is not getting out of the penalty box"
                << std::endl;
      return;
    }
  }

  movePlayer(roll);
  std::cout << players_[currentPlayer_].name << "'s new location is "
            << players_[currentPlayer_].state.place << std::endl;
  std::cout << "The category is " << ToStringView(currentCategory()) << std::endl;
  askQuestion();
}

void Game::movePlayer(int n_steps)
{
  players_[currentPlayer_].state.place = (players_[currentPlayer_].state.place + n_steps) % 12;
}

void Game::askQuestion()
{
  auto& questionGroup = questionPool_[currentCategory()];
  std::cout << questionGroup.front() << std::endl;
  questionGroup.pop_front();
}

Category Game::currentCategory()
{
  switch (players_[currentPlayer_].state.place % 4) {
    case 0: return Category::Pop;
    case 1: return Category::Science;
    case 2: return Category::Sports;
    default: return Category::Rock;
  }
}

bool Game::wasCorrectlyAnswered()
{
  if (players_[currentPlayer_].state.inPenaltyBox) {
    if (isGettingOutOfPenaltyBox_) {
      std::cout << "Answer was correct!!!!" << std::endl;
    } else {
      makeNextPlayerTheCurrent();
      return true;
    }
  } else {
    std::cout << "Answer was corrent!!!!" << std::endl;
  }
  players_[currentPlayer_].state.purse++;
  std::cout << players_[currentPlayer_].name << " now has " << players_[currentPlayer_].state.purse
            << " Gold Coins." << std::endl;

  const bool didPlayerWin = players_[currentPlayer_].state.purse == 6;
  makeNextPlayerTheCurrent();

  return !didPlayerWin;
}

void Game::makeNextPlayerTheCurrent()
{
  currentPlayer_ = (currentPlayer_ + 1) % players_.size();
}

bool Game::wrongAnswer()
{
  std::cout << "Question was incorrectly answered" << std::endl;
  std::cout << players_[currentPlayer_].name + " was sent to the penalty box" << std::endl;
  players_[currentPlayer_].state.inPenaltyBox = true;

  makeNextPlayerTheCurrent();
  return true;
}
