#pragma once

#include "Game.h"

#include <string>

struct Player {
  struct State {
    State(int field, int coins, bool inPenaltyBox)
        : field(field), coins(coins), inPenaltyBox(inPenaltyBox)
    {
    }
    int field;
    int coins;
    bool inPenaltyBox;
  };

  Player(std::string name, State state) : name(std::move(name)), state(state) {}
  std::string name;
  State state;
};
using QuestionPool = std::unordered_map<Category, std::list<std::string>>;

class TriviaGameTurn : public GameTurn {
 public:
  TriviaGameTurn(Player& player, QuestionPool& questionPool, std::ostream& logger);

  virtual int rollDice() const override;
  virtual std::optional<int> movePlayer(int roll) override;
  virtual std::optional<Question> readQuestion(int location) override;
  virtual Answer askQuestion(Question question) override;
  virtual bool isAnswerCorrect(Answer answer) override;
  virtual void onCorrectAnswer() override;
  virtual void onIncorrectAnswer() override;

 private:
  std::optional<std::string> nextQuestion(Category category);

  Player& player_;
  QuestionPool& questionPool_;
  std::ostream& logger_;
};

class TriviaGame : public Game {
 public:
  static std::optional<TriviaGame> Create(std::vector<std::string> playerNames,
                                          QuestionPool questionPool,
                                          std::ostream& logger);

 private:
  TriviaGame(std::vector<Player> players, QuestionPool questionPool, std::ostream& logger);

  virtual std::unique_ptr<GameTurn> newTurn(int playerId) override;
  virtual bool didPlayerWin(int playerId) const override;

  std::vector<Player> players_;
  QuestionPool questionPool_;
  std::ostream& logger_;
};
