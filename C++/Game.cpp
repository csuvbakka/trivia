﻿#include "Game.h"
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

Game::Game(std::vector<Player> players, QuestionPool questionPool)
    : players_(std::move(players))
    , questionPool_(std::move(questionPool))
    , currentPlayer_(std::prev(players_.begin()))
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

void Game::run()
{
  bool notAWinner = true;
  do {
    updateCurrentPlayer();
    if (rollWithNextPlayer(rand() % 5 + 1))
      readQuestion();

    if (rand() % 9 == 7) {
      notAWinner = answer(false);
    } else {
      notAWinner = answer(true);
    }
  } while (notAWinner);
}

bool Game::isPlayable()
{
  return (players_.size() >= 2);
}

std::optional<int> Game::rollWithNextPlayer(int roll)
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
      return std::nullopt;
    }
  }

  currentPlayer_->state.field = (currentPlayer_->state.field + roll) % 12;
  std::cout << currentPlayer_->name << "'s new location is " << currentPlayer_->state.field << "\n";
  return currentPlayer_->state.field;
}

void Game::readQuestion()
{
  const auto category = categoryForField(currentPlayer_->state.field);
  const auto question = nextQuestion(category);

  std::cout << "The category is " << ToStringView(category) << "\n";
  std::cout << question << "\n";
}

std::string Game::nextQuestion(Category category)
{
  auto& questionGroup = questionPool_[category];
  auto question       = questionGroup.front();
  questionGroup.pop_front();
  return question;
}

void Game::updateCurrentPlayer()
{
  ++currentPlayer_;
  if (currentPlayer_ == players_.end())
    currentPlayer_ = players_.begin();
}

bool Game::answer(bool isCorrect)
{
  if (!isCorrect) {
    std::cout << "Question was incorrectly answered\n";
    std::cout << currentPlayer_->name + " was sent to the penalty box\n";
    currentPlayer_->state.inPenaltyBox = true;

    return true;
  } else {
    if (currentPlayer_->state.inPenaltyBox) {
      if (isGettingOutOfPenaltyBox_) {
        std::cout << "Answer was correct!!!!\n";
      } else {
        return true;
      }
    } else {
      std::cout << "Answer was corrent!!!!\n";
    }
    currentPlayer_->state.coins++;
    std::cout << currentPlayer_->name << " now has " << currentPlayer_->state.coins
              << " Gold Coins.\n";

    const bool didPlayerWin = currentPlayer_->state.coins == 6;

    return !didPlayerWin;
  }
}
