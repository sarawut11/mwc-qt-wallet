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

namespace core {

class WalletApp : public QApplication {
Q_OBJECT
public:
    WalletApp(int &argc, char **argv);

    // app is done with init process, basic objects should be good to go
    void reportAppAsInitialized();

    // true of app object is created, so qt engine is initialized
    static bool isAppCreated();

    // true if initialized
    static bool isAppInitialized();

    static bool isExiting();

    static void startExiting();
protected:
    virtual bool notify(QObject *receiver, QEvent *event) override;
};

}

#endif //MWC_QT_WALLET_WALLETAPP_H
