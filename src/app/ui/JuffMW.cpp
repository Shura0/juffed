#include <QDebug>

#include "JuffMW.h"
#include "Log.h"

#include "AboutDlg.h"
#include "AppInfo.h"
#include "CommandStorage.h"
#include "Document.h"
#include "Functions.h"
#include "License.h"
#include "MainSettings.h"
#include "MessageWidget.h"
#include "SelectFilesDlg.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QVBoxLayout>

struct Helper {
	QString name;
	QString urlTitle;
	QString urlHref;
	QString contribution;
	Helper (const QString& nm = "", const QString& urlTtl = "", const QString& urlHrf = "", const QString& contr = "") {
		name = nm;
		urlTitle = urlTtl;
		urlHref = urlHrf;
		contribution = contr;
	}
};

AboutDlg* createAboutDlg(QWidget* parent) {
	AboutDlg* dlg = new AboutDlg(parent);
	dlg->setWindowTitle(QObject::tr("About"));
//	dlg->setProgramName(AppInfo::name() + " v" + AppInfo::version());
	dlg->setProgramName(AppInfo::name());
	QString text = QString("   %1   <br><br>   Copyright &copy; 2007-2009 Mikhail Murzin   <br><br><a href=\"http://sourceforge.net/projects/juffed/\">http://sourceforge.net/projects/juffed/</a>").arg(QObject::tr("Advanced text editor"));
	QString auth("<br>&nbsp;Mikhail Murzin a.k.a. Mezomish<br>&nbsp;&nbsp;<a href='mailto:mezomish@gmail.com'>mezomish@gmail.com</a>");
	QList<Helper> helpers;
	helpers << Helper("Eugene Pivnev", "ti.eugene@gmail.com", "mailto:ti.eugene@gmail.com", QObject::tr("Packaging"))
			<< Helper("David Stegbauer", "daaste@gmail.com", "mailto:daaste@gmail.com", QObject::tr("Patches"))
			<< Helper("Jarek", "ajep9691@wp.pl", "mailto:ajep9691@wp.pl", QObject::tr("Polish translation"))
			<< Helper("Michael Gangolf", "miga@migaweb.de", "mailto:miga@migaweb.de", QObject::tr("German translation"))
			<< Helper("Marc Dumoulin", "shadosan@gmail.com", "mailto:shadosan@gmail.com", QObject::tr("French translation"))
			<< Helper("Pavel Fric", "http://fripohled.blogspot.com/", "http://fripohled.blogspot.com/", QObject::tr("Czech translation"))
			<< Helper("Giuliano S. Nascimento", "giusoft.tecnologia@gmail.com", "mailto:giusoft.tecnologia@gmail.com", QObject::tr("Brazilian Portuguese translation"))
			<< Helper("YANG Weichun", "htyoung@163.com", "mailto:htyoung@163.com", QObject::tr("Chinese Simplified translation"))
			<< Helper("\"SoftIcon\"", "http://softicon.ru/", "http://softicon.ru/", QObject::tr("Application icon"))
			<< Helper("Evgeny Muravjev Studio", "http://emuravjev.ru/", "http://emuravjev.ru/", QObject::tr("Website"))
	;

	QString thanks;
	foreach(Helper helper, helpers) {
		thanks += QString("&nbsp;%1<br>").arg(helper.name);
		thanks += QString("&nbsp;&nbsp;<a href='%1'>%2</a><br>").arg(helper.urlHref).arg(helper.urlTitle);
		thanks += QString("&nbsp;&nbsp;%1<br><br>").arg(helper.contribution);
	}

	dlg->setText(text);
	dlg->setAuthors(auth);
	dlg->setThanks(thanks);
	dlg->setLicense(License::licenseText, false);
	dlg->resize(550, 350);
	dlg->setIcon(QIcon(":juffed_48.png"));
	
	return dlg;
}

JuffMW::JuffMW() : QMainWindow() {
	resize(800, 700);

	mainWidget_ = new QWidget();
	vBox_ = new QVBoxLayout();
	vBox_->setContentsMargins(0, 0, 0, 0);
	mainWidget_->setLayout(vBox_);
	setCentralWidget(mainWidget_);
	
	aboutDlg_ = createAboutDlg(this);
}

void JuffMW::setViewer(QWidget* w) {
	vBox_->addWidget(w);
}

void JuffMW::about() {
	aboutDlg_->exec();
}

void JuffMW::aboutQt() {
	QMessageBox::aboutQt(this, tr("About Qt"));
}

QString JuffMW::getOpenFileName(const QString& dir, const QString& filters) {
	return QFileDialog::getOpenFileName(this, tr("Open file"), dir, filters);
}

QStringList JuffMW::getOpenFileNames(const QString& dir, const QString& filters) {
	return QFileDialog::getOpenFileNames(this, tr("Open files"), dir, filters);
}

QString JuffMW::getSaveFileName(const QString& curFileName, const QString& filters) {
	QString fileName;
	if ( !curFileName.isEmpty() && !Juff::isNoname(curFileName) ) {
		fileName = curFileName;
	}
	return QFileDialog::getSaveFileName(this, tr("Save %1 as...").arg(Juff::docTitle(curFileName, false)), fileName, filters);
}

QString JuffMW::getSavePrjName(const QString& title) {
	// TODO :
	QString dir = "";
	return QFileDialog::getSaveFileName(this, title, dir, "XML JuffEd Project Files (*.xml)");
}

int JuffMW::getGotoLineNumber(int lineCount) {
	bool ok = false;
	int line = QInputDialog::getInteger(this, tr("Go to line"), 
			tr("Go to line") + QString(" (1 - %1):").arg(lineCount), 
			1, 1, lineCount, 1, &ok);
	if ( ok )
		return line - 1;
	else
		return -1;
}

int JuffMW::askForSave(const QString& fileName) {
	QString str = tr("The document ") + fileName;
	str += tr(" has been modified.\nDo you want to save your changes?");
	return QMessageBox::warning(this, tr("Close document"),
			str, QMessageBox::Save | QMessageBox::Discard
			| QMessageBox::Cancel, QMessageBox::Save);
}

bool JuffMW::askForSave(const QStringList& filesIn, QStringList& filesOut) {
	LOGGER;
	
	filesOut.clear();
	
	SelectFilesDlg dlg(filesIn, this);
	if ( dlg.exec() == QDialog::Accepted ) {
		filesOut = dlg.checkedFiles();
		return true;
	}
	else {
		return false;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Information display

void JuffMW::addStatusWidget(QWidget* w) {
	statusBar()->addWidget(w);
}

void JuffMW::message(const QIcon& icon, const QString& title, const QString& message, int timeout) {
	MessageWidget* msg = new MessageWidget(QIcon(), title, message, timeout, this);
	vBox_->insertWidget(0, msg);
	vBox_->setStretchFactor(msg, 0);
//	vBox_->addWidget(msg);
}



////////////////////////////////////////////////////////////////////////////////

void JuffMW::closeEvent(QCloseEvent* e) {
	LOGGER;
	
	bool confirmed = true;
	emit closeRequested(confirmed);
	if ( confirmed ) {
//		MainSettings::setWindowRect(geometry());
		e->accept();
	}
	else {
		e->ignore();
	}
}