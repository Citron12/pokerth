//
// C++ Implementation: gamelobbydialogimpl
//
// Description: 
//
//
// Author: FThauer FHammer LMay <webmaster@pokerth.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "gamelobbydialogimpl.h"
#include "session.h"
#include "configfile.h"
#include "gamedata.h"
#include <net/socket_msg.h>

gameLobbyDialogImpl::gameLobbyDialogImpl(QWidget *parent, ConfigFile *c)
 : QDialog(parent), myW(NULL), myConfig(c), mySession(NULL), currentGameName(""), isAdmin(false)
{
    setupUi(this);

	connect( pushButton_CreateGame, SIGNAL( clicked() ), this, SLOT( createGame() ) );
	connect( pushButton_JoinGame, SIGNAL( clicked() ), this, SLOT( joinGame() ) );
	connect( treeWidget_GameList, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*) ), this, SLOT( gameSelected(QTreeWidgetItem*, QTreeWidgetItem*) ) );
	connect( pushButton_StartGame, SIGNAL( clicked() ), this, SLOT( startGame() ) );
	connect( pushButton_Kick, SIGNAL( clicked() ), this, SLOT( kickPlayer() ) );
	connect( pushButton_Leave, SIGNAL( clicked() ), this, SLOT( leaveGame() ) );
	connect( treeWidget_connectedPlayers, SIGNAL( currentItemChanged ( QTreeWidgetItem*, QTreeWidgetItem*) ), this, SLOT( playerSelected(QTreeWidgetItem*, QTreeWidgetItem*) ) );
	connect( lineEdit_ChatInput, SIGNAL( returnPressed () ), this, SLOT( sendChatMessage() ) );

	clearDialog();
}

void gameLobbyDialogImpl::exec()
{
	QDialog::exec();
	clearDialog();
}


gameLobbyDialogImpl::~gameLobbyDialogImpl()
{
}

void gameLobbyDialogImpl::setSession(Session *session)
{
	mySession = session;
}

void gameLobbyDialogImpl::createGame()
{
	assert(mySession);

	myCreateInternetGameDialog = new createInternetGameDialogImpl(this, myConfig);
	myCreateInternetGameDialog->exec();
	
	if (myCreateInternetGameDialog->result() == QDialog::Accepted ) {

		GameData gameData;
		// Set Game Data
		gameData.maxNumberOfPlayers = myCreateInternetGameDialog->spinBox_quantityPlayers->value();
		gameData.startMoney = myCreateInternetGameDialog->spinBox_startCash->value();
		gameData.smallBlind = myCreateInternetGameDialog->spinBox_smallBlind->value();
		gameData.handsBeforeRaise = myCreateInternetGameDialog->spinBox_handsBeforeRaiseSmallBlind->value();
		gameData.guiSpeed = myCreateInternetGameDialog->spinBox_gameSpeed->value();
		gameData.playerActionTimeoutSec = myCreateInternetGameDialog->spinBox_netTimeOutPlayerAction->value();

		QString gameString(tr("game"));
		currentGameName = QString::fromUtf8(myConfig->readConfigString("MyName").c_str()) + QString("'s "+ gameString);

		label_SmallBlind->setText(QString::number(gameData.smallBlind));
		label_StartCash->setText(QString::number(gameData.startMoney));
		label_MaximumNumberOfPlayers->setText(QString::number(gameData.maxNumberOfPlayers));
		label_HandsToRaiseSmallBlind->setText(QString::number(gameData.handsBeforeRaise));
		label_TimeoutForPlayerAction->setText(QString::number(gameData.playerActionTimeoutSec));

		mySession->clientCreateGame(gameData, myConfig->readConfigString("MyName") + "'s "+ gameString.toUtf8().constData(), myCreateInternetGameDialog->lineEdit_Password->text().toUtf8().constData());
	}
}

void gameLobbyDialogImpl::joinGame()
{
	QTreeWidgetItem *item = treeWidget_GameList->currentItem();
	if (item)
	{
		assert(mySession);

		int isPrivate = 1;
		//if private ask for password
		if(isPrivate) {

			bool ok;
			QString password = QInputDialog::getText(this, tr("Join a private Game"),
								tr("You like to join a private game. Please enter the password!"), QLineEdit::Password, (""), &ok);
			if (ok && !password.isEmpty()) {
// 				std::cout << password.toStdString() << "\n";
			}
				
		}

		mySession->clientJoinGame(item->data(0, Qt::UserRole).toUInt(), "");
	}
}

void gameLobbyDialogImpl::refresh(int actionID) {

	if (actionID == MSG_NET_GAME_CLIENT_START)
	{
		QTimer::singleShot(500, this, SLOT(accept()));
	}
}

void gameLobbyDialogImpl::removedFromGame(int reason)
{
	clearDialog();
}

void gameLobbyDialogImpl::gameSelected(QTreeWidgetItem* item, QTreeWidgetItem*)
{
	if (item)
	{
		pushButton_JoinGame->setEnabled(true);

		currentGameName = item->text(0);

		groupBox_GameInfo->setEnabled(true);
		groupBox_GameInfo->setTitle(tr("Game Info") + " - " + currentGameName);

		assert(mySession);
		GameInfo info(mySession->getClientGameInfo(item->data(0, Qt::UserRole).toUInt()));
		label_SmallBlind->setText(QString::number(info.data.smallBlind));
		label_StartCash->setText(QString::number(info.data.startMoney));
		label_MaximumNumberOfPlayers->setText(QString::number(info.data.maxNumberOfPlayers));
		label_HandsToRaiseSmallBlind->setText(QString::number(info.data.handsBeforeRaise));
		label_TimeoutForPlayerAction->setText(QString::number(info.data.playerActionTimeoutSec));

		treeWidget_connectedPlayers->clear();
		PlayerIdList::const_iterator i = info.players.begin();
		PlayerIdList::const_iterator end = info.players.end();
		while (i != end)
		{
			PlayerInfo info(mySession->getClientPlayerInfo(*i));
			addConnectedPlayer(*i, QString::fromUtf8(info.playerName.c_str()), PLAYER_RIGHTS_NORMAL);
			++i;
		}
	}
}

void gameLobbyDialogImpl::updateGameItem(QTreeWidgetItem *item, unsigned gameId)
{
	assert(mySession);
	GameInfo info(mySession->getClientGameInfo(gameId));

	item->setData(0, Qt::UserRole, gameId);
	item->setData(0, Qt::DisplayRole, QString::fromUtf8(info.name.c_str()));

	QString playerStr;
	playerStr.sprintf("%u/%u", info.players.size(), info.data.maxNumberOfPlayers);
	item->setData(1, Qt::DisplayRole, playerStr);

	if (info.isPasswordProtected)
		item->setData(2, Qt::DisplayRole, "X");
}

void gameLobbyDialogImpl::addGame(unsigned gameId)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_GameList, 0);

	updateGameItem(item, gameId);
}

void gameLobbyDialogImpl::removeGame(unsigned gameId)
{
	QTreeWidgetItemIterator it(treeWidget_GameList);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == gameId)
		{
			treeWidget_GameList->takeTopLevelItem(treeWidget_GameList->indexOfTopLevelItem(*it));
			break;
		}
		++it;
	}
}

void gameLobbyDialogImpl::gameAddPlayer(unsigned gameId, unsigned playerId)
{
	QTreeWidgetItem *item = treeWidget_GameList->currentItem();
	if (item && item->data(0, Qt::UserRole) == gameId)
	{
		assert(mySession);
		PlayerInfo info(mySession->getClientPlayerInfo(playerId));
		addConnectedPlayer(playerId, QString::fromUtf8(info.playerName.c_str()), PLAYER_RIGHTS_NORMAL);
	}

	QTreeWidgetItemIterator it(treeWidget_GameList);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == gameId)
		{
			updateGameItem(*it, gameId);
			break;
		}
		++it;
	}
}

void gameLobbyDialogImpl::gameRemovePlayer(unsigned gameId, unsigned playerId)
{
	QTreeWidgetItem *item = treeWidget_GameList->currentItem();
	if (item && item->data(0, Qt::UserRole) == gameId)
	{
		assert(mySession);
		PlayerInfo info(mySession->getClientPlayerInfo(playerId));
		removePlayer(playerId, QString::fromUtf8(info.playerName.c_str()));
	}

	QTreeWidgetItemIterator it(treeWidget_GameList);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == gameId)
		{
			updateGameItem(*it, gameId);
			break;
		}
		++it;
	}
}

void gameLobbyDialogImpl::clearDialog()
{
	groupBox_GameInfo->setTitle(tr("Game Info"));
	groupBox_GameInfo->setEnabled(false);
	currentGameName = "";

	label_SmallBlind->setText("");
	label_StartCash->setText("");
	label_MaximumNumberOfPlayers->setText("");
	label_HandsToRaiseSmallBlind->setText("");
	label_TimeoutForPlayerAction->setText("");

	treeWidget_GameList->clear();
	treeWidget_GameList->show();
	treeWidget_connectedPlayers->clear();
	
	pushButton_Leave->hide();
	pushButton_Kick->hide();
	pushButton_Kick->setEnabled(false);
	pushButton_StartGame->hide();
	checkBox_fillUpWithComputerOpponents->hide();
	pushButton_CreateGame->show();
	pushButton_JoinGame->show();
	pushButton_JoinGame->setEnabled(false);

	treeWidget_GameList->setColumnWidth(0,250);
	treeWidget_GameList->setColumnWidth(1,75);
	treeWidget_GameList->setColumnWidth(2,70);

	lineEdit_ChatInput->setFocus();
}

void gameLobbyDialogImpl::checkPlayerQuantity() {

	if(isAdmin){
		pushButton_Kick->show();
		pushButton_StartGame->show();
		checkBox_fillUpWithComputerOpponents->show();
		
		if (treeWidget_connectedPlayers->topLevelItemCount() >= 2) {
			pushButton_StartGame->setEnabled(true);
		}
		else {
			pushButton_StartGame->setEnabled(false);
		}
	}
}

void gameLobbyDialogImpl::joinedNetworkGame(unsigned playerId, QString playerName, int rights) {

	// Update dialog
	gameModeDialogUpdate();

	isAdmin = rights == PLAYER_RIGHTS_ADMIN;
	addConnectedPlayer(playerId, playerName, rights);
}


void gameLobbyDialogImpl::addConnectedPlayer(unsigned playerId, QString playerName, int rights) {

	QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_connectedPlayers, 0);
	item->setData(0, Qt::UserRole, playerId);
	item->setData(0, Qt::DisplayRole, playerName);

/*	assert(mySession);
	GameInfo info = mySession->getClientGameInfo(currentGameName.toUtf8().constData());

	if(treeWidget_connectedPlayers->topLevelItemCount() != info.data.maxNumberOfPlayers) {
		myW->getMySDLPlayer()->playSound("playerconnected", 0);
	}
	else {
		myW->getMySDLPlayer()->playSound("onlinegameready", 0);
	}*/

	checkPlayerQuantity();
}

void gameLobbyDialogImpl::updatePlayer(unsigned playerId, QString newPlayerName) {

	QTreeWidgetItemIterator it(treeWidget_connectedPlayers);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == playerId)
		{
			(*it)->setData(0, Qt::DisplayRole, newPlayerName);
			break;
		}
		++it;
	}
}

void gameLobbyDialogImpl::removePlayer(unsigned playerId, QString) {

	QTreeWidgetItemIterator it(treeWidget_connectedPlayers);
	while (*it) {
		if ((*it)->data(0, Qt::UserRole) == playerId)
		{
			treeWidget_connectedPlayers->takeTopLevelItem(treeWidget_connectedPlayers->indexOfTopLevelItem(*it));
			break;
		}
		++it;
	}

	checkPlayerQuantity();
}

void gameLobbyDialogImpl::gameModeDialogUpdate() {
	groupBox_GameInfo->setEnabled(true);
	groupBox_GameInfo->setTitle(currentGameName);
	treeWidget_GameList->clear();
	treeWidget_GameList->hide();
	treeWidget_connectedPlayers->clear();
	pushButton_CreateGame->hide();
	pushButton_JoinGame->hide();
	pushButton_Leave->show();
}

void gameLobbyDialogImpl::playerSelected(QTreeWidgetItem* item, QTreeWidgetItem*) {

	if (item)
		pushButton_Kick->setEnabled(isAdmin);
}

void gameLobbyDialogImpl::startGame() {

	assert(mySession);
	mySession->sendStartEvent(checkBox_fillUpWithComputerOpponents->isChecked());
}

void gameLobbyDialogImpl::leaveGame() {

	assert(mySession);
	mySession->sendLeaveCurrentGame();
}

void gameLobbyDialogImpl::kickPlayer() {

	QTreeWidgetItem *item = treeWidget_connectedPlayers->currentItem();
	if (item)
	{
		QString playerName = item->text(0);
		if(playerName == QString::fromUtf8(myConfig->readConfigString("MyName").c_str())) {
			{ QMessageBox::warning(this, tr("Server Error"),
					tr("You should not kick yourself from this game!"),
					QMessageBox::Close); }
		}
		else {
			assert(mySession);
			mySession->kickPlayer(item->data(0, Qt::UserRole).toUInt());
		}
	}
	pushButton_Kick->setEnabled(false);
}

void gameLobbyDialogImpl::sendChatMessage() { /*myChat->sendMessage();*/ }
