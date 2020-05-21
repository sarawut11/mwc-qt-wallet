// Copyright 2020 The MWC Developers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef MWC_QT_WALLET_WALLETAPP_H
#define MWC_QT_WALLET_WALLETAPP_H

#include "QApplication"

namespace state {
class StateMachine;
}

namespace core {

class WalletApp : public QApplication {
Q_OBJECT
public:
    WalletApp(int &argc, char **argv);

    void setStateMachine(state::StateMachine * _stateMachine) {stateMachine = _stateMachine;}

protected:
    virtual bool notify(QObject *receiver, QEvent *event) override;

private:
    state::StateMachine * stateMachine = nullptr;
};

}

#endif //MWC_QT_WALLET_WALLETAPP_H