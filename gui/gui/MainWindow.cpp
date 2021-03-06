/* Copyright (C) 2015-2016 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"
#include "model/FolderModel.h"
#include "ui_MainWindow.h"
#include <icons/GUIIconProvider.h>
#include "FolderProperties.h"

#ifdef Q_OS_MAC
void qt_mac_set_dock_menu(QMenu *menu);
#endif

MainWindow::MainWindow(Client& client, QWidget* parent) :
		QMainWindow(parent),
		client_(client),
		ui(std::make_unique<Ui::MainWindow>()) {
	ui->setupUi(this);
	/* Initializing models */
	folder_model_ = std::make_unique<FolderModel>(this);
	ui->treeView->setModel(folder_model_.get());
	ui->treeView->header()->setStretchLastSection(false);
	ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

	/* Initializing dialogs */
	settings_ = new Settings(this);
	connect(settings_, &Settings::newConfigIssued, this, &MainWindow::newConfigIssued);

	add_folder_ = new AddFolder(this);
	connect(add_folder_, &AddFolder::folderAdded, this, &MainWindow::folderAdded);

	connect(ui->treeView, &QAbstractItemView::doubleClicked, this, &MainWindow::handleOpenFolderProperties);

	init_actions();
	init_tray();
	init_toolbar();
	retranslateUi();
}

MainWindow::~MainWindow() {}

void MainWindow::retranslateUi() {
	show_main_window_action->setText(tr("Show Librevault window"));
	open_website_action->setText(tr("Open Librevault website"));
	show_settings_window_action->setText(tr("Settings"));
	show_settings_window_action->setToolTip(tr("Open Librevault settings"));
	exit_action->setText(tr("Quit Librevault"));    // TODO: Apply: https://ux.stackexchange.com/questions/50893/do-we-exit-quit-or-close-an-application
	exit_action->setToolTip("Stop synchronization and exit Librevault application");
	new_folder_action->setText(tr("New folder"));
	new_folder_action->setToolTip(tr("Add new folder for synchronization"));
	delete_folder_action->setText(tr("Delete"));
	delete_folder_action->setToolTip(tr("Delete folder"));

	ui->retranslateUi(this);
	settings_->retranslateUi();
}

void MainWindow::handleControlJson(QJsonObject state_json) {
	settings_->handleControlJson(state_json);
	folder_model_->handleControlJson(state_json);
}

void MainWindow::openWebsite() {
	QDesktopServices::openUrl(QUrl("https://librevault.com"));
}

void MainWindow::tray_icon_activated(QSystemTrayIcon::ActivationReason reason) {
#ifndef Q_OS_MAC
	if(reason != QSystemTrayIcon::Context) show_main_window_action->trigger();
#endif
}

void MainWindow::handleRemoveFolder() {
	QMessageBox* confirmation_box = new QMessageBox(
		QMessageBox::Warning,
		tr("Remove folder from Librevault?"),
		tr("This folder will be removed from Librevault and no longer synced with other peers. Existing folder contents will not be altered."),
		QMessageBox::Ok | QMessageBox::Cancel,
		this
	);

	confirmation_box->setDefaultButton(QMessageBox::Cancel);
	confirmation_box->setWindowModality(Qt::WindowModal);

	if(confirmation_box->exec() == QMessageBox::Ok) {
		auto selection_model = ui->treeView->selectionModel()->selectedRows();
		for(auto model_index : selection_model) {
			qDebug() << model_index;
			QString secret = folder_model_->data(model_index, FolderModel::SecretRole).toString();
			qDebug() << secret;
			emit folderRemoved(secret);
		}
	}
}

void MainWindow::handleOpenFolderProperties(const QModelIndex &index) {
	QByteArray hash = folder_model_->data(index, FolderModel::HashRole).toByteArray();
	folder_model_->getFolderDialog(hash)->show();
}

void MainWindow::changeEvent(QEvent* e) {
	QMainWindow::changeEvent(e);
	switch(e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void MainWindow::init_actions() {
	show_main_window_action = new QAction(this);
	connect(show_main_window_action, &QAction::triggered, this, &QMainWindow::show);
	connect(show_main_window_action, &QAction::triggered, this, &QMainWindow::activateWindow);
	connect(show_main_window_action, &QAction::triggered, this, &QMainWindow::raise);

	open_website_action = new QAction(this);
	connect(open_website_action, &QAction::triggered, this, &MainWindow::openWebsite);

	show_settings_window_action = new QAction(this);
	show_settings_window_action->setIcon(GUIIconProvider::get_instance()->get_icon(GUIIconProvider::SETTINGS));
	show_settings_window_action->setMenuRole(QAction::PreferencesRole);
	connect(show_settings_window_action, &QAction::triggered, settings_, &QDialog::show);

	exit_action = new QAction(this);
	QIcon exit_action_icon; // TODO
	exit_action->setIcon(exit_action_icon);
	connect(exit_action, &QAction::triggered, this, &QCoreApplication::quit);

	new_folder_action = new QAction(this);
	new_folder_action->setIcon(GUIIconProvider::get_instance()->get_icon(GUIIconProvider::FOLDER_ADD));
	connect(new_folder_action, &QAction::triggered, add_folder_, &AddFolder::show);

	delete_folder_action = new QAction(this);
	delete_folder_action->setIcon(GUIIconProvider::get_instance()->get_icon(GUIIconProvider::FOLDER_DELETE));
	delete_folder_action->setShortcut(Qt::Key_Delete);
	connect(delete_folder_action, &QAction::triggered, this, &MainWindow::handleRemoveFolder);
}

void MainWindow::init_toolbar() {
#if defined(Q_OS_MAC) || defined(Q_OS_WIN)
	ui->toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
#endif
	ui->toolBar->addAction(new_folder_action);
	ui->toolBar->addAction(delete_folder_action);
	ui->toolBar->addSeparator();
	ui->toolBar->addAction(show_settings_window_action);
}

void MainWindow::init_tray() {
	// Context menu
	tray_context_menu.addAction(show_main_window_action);
	tray_context_menu.addAction(open_website_action);
	tray_context_menu.addSeparator();
	tray_context_menu.addAction(show_settings_window_action);
	tray_context_menu.addSeparator();
	tray_context_menu.addAction(exit_action);
#ifdef Q_OS_MAC
	qt_mac_set_dock_menu(&tray_context_menu);
#endif
	tray_icon.setContextMenu(&tray_context_menu);

	// Icon itself
	connect(&tray_icon, &QSystemTrayIcon::activated, this, &MainWindow::tray_icon_activated);

	tray_icon.setIcon(GUIIconProvider::get_instance()->get_icon(GUIIconProvider::TRAYICON));   // FIXME: Temporary measure. Need to display "sync" icon here. Also, theming support.
	tray_icon.setToolTip(tr("Librevault"));

	tray_icon.show();
}
