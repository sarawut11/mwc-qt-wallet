#ifndef SEND2_ONLINE_H
#define SEND2_ONLINE_H

#include "../core/navwnd.h"
#include "../wallet/wallet.h"

namespace Ui {
class SendOnline;
}

namespace state {
class SendOnline;
}

namespace core {
class SendCoinsParams;
}

namespace wnd {

class SendOnline : public core::NavWnd
{
    Q_OBJECT

public:
    explicit SendOnline(QWidget *parent, state::SendOnline * state );
    virtual ~SendOnline() override;

    void sendRespond( bool success, const QStringList & errors );

private slots:
    void on_contactsButton_clicked();
    void on_allAmountButton_clicked();
    void on_sendButton_clicked();

    void on_settingsButton2_clicked();

private:
    Ui::SendOnline *ui;
    state::SendOnline * state = nullptr;
    QVector<wallet::AccountInfo> accountInfo;
    QVector<wallet::WalletContact> contacts;
};

extern QString generateAmountErrorMsg( long mwcAmount, const wallet::AccountInfo & acc, const core::SendCoinsParams & sendParams );


}


#endif // SEND2_ONLINE_H
