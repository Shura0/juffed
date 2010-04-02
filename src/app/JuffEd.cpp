#include <QDebug>

#include "JuffEd.h"

#include "AppInfo.h"
#include "CharsetSettings.h"
#include "CommandStorage.h"
#include "Document.h"
#include "Functions.h"
#include "IconManager.h"
#include "Log.h"
#include "MainSettings.h"
#include "Project.h"
#include "Settings.h"
#include "StatusLabel.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QTranslator>

// TODO : remove this to a plugin!
#include <QDockWidget>


JuffEd::JuffEd() : Juff::PluginNotifier(), Juff::DocHandlerInt(), pluginMgr_(this, this) {
	LOGGER;
	
	Settings::read();
	QString prjName = MainSettings::get(MainSettings::LastProject);
	prj_ = new Juff::Project(prjName);
	
	charsetMenu_ = openWithCharsetMenu_ = setCharsetMenu_ = NULL;
	
/*	QString lng = AppInfo::language();
	QTranslator translator;
	if ( translator.load("juffed_" + lng, AppInfo::translationPath()) ) {
		if ( !translator.isEmpty() ) {
			qApp->installTranslator(&translator);
		}
	}*/
	
	mw_ = new JuffMW();
	
	tree_ = new ProjectTree(this);
	tree_->setProject(prj_);
	QDockWidget* dock = new QDockWidget();
	dock->setWidget(tree_);
	mw_->addDockWidget(Qt::LeftDockWidgetArea, dock);
	
	foreach (QMenu* menu, menus_.values()) {
		mw_->menuBar()->addMenu(menu);
	}
	
	// actions
	CommandStorage* st = CommandStorage::instance();
	connect(st->action(Juff::FileNew), SIGNAL(triggered()), this, SLOT(slotFileNew()));
	connect(st->action(Juff::FileOpen), SIGNAL(triggered()), this, SLOT(slotFileOpen()));
	connect(st->action(Juff::FileRename), SIGNAL(triggered()), this, SLOT(slotFileRename()));
	connect(st->action(Juff::FileSave), SIGNAL(triggered()), this, SLOT(slotFileSave()));
	connect(st->action(Juff::FileSaveAs), SIGNAL(triggered()), this, SLOT(slotFileSaveAs()));
	connect(st->action(Juff::FileSaveAll), SIGNAL(triggered()), this, SLOT(slotFileSaveAll()));
	connect(st->action(Juff::FileReload), SIGNAL(triggered()), this, SLOT(slotFileReload()));
	connect(st->action(Juff::FileRename), SIGNAL(triggered()), this, SLOT(slotFileRename()));
	connect(st->action(Juff::FileClose), SIGNAL(triggered()), this, SLOT(slotFileClose()));
	connect(st->action(Juff::FileCloseAll), SIGNAL(triggered()), this, SLOT(slotFileCloseAll()));
	connect(st->action(Juff::FilePrint), SIGNAL(triggered()), this, SLOT(slotFilePrint()));
	connect(st->action(Juff::FileExit), SIGNAL(triggered()), this, SLOT(slotFileExit()));
	
	connect(st->action(Juff::PrjNew), SIGNAL(triggered()), this, SLOT(slotPrjNew()));
	connect(st->action(Juff::PrjOpen), SIGNAL(triggered()), this, SLOT(slotPrjOpen()));
//	connect(st->action(Juff::PrjRename), SIGNAL(triggered()), this, SLOT(slotPrjRename()));
	connect(st->action(Juff::PrjClose), SIGNAL(triggered()), this, SLOT(slotPrjClose()));
	connect(st->action(Juff::PrjSaveAs), SIGNAL(triggered()), this, SLOT(slotPrjSaveAs()));
	connect(st->action(Juff::PrjAddFile), SIGNAL(triggered()), this, SLOT(slotPrjAddFile()));
	
	connect(st->action(Juff::EditUndo), SIGNAL(triggered()), this, SLOT(slotEditUndo()));
	connect(st->action(Juff::EditRedo), SIGNAL(triggered()), this, SLOT(slotEditRedo()));
	connect(st->action(Juff::EditCut), SIGNAL(triggered()), this, SLOT(slotEditCut()));
	connect(st->action(Juff::EditCopy), SIGNAL(triggered()), this, SLOT(slotEditCopy()));
	connect(st->action(Juff::EditPaste), SIGNAL(triggered()), this, SLOT(slotEditPaste()));
	
	connect(st->action(Juff::EditFind), SIGNAL(triggered()), this, SLOT(slotFind()));
	connect(st->action(Juff::EditFindNext), SIGNAL(triggered()), this, SLOT(slotFindNext()));
	connect(st->action(Juff::EditFindPrev), SIGNAL(triggered()), this, SLOT(slotFindPrev()));
	connect(st->action(Juff::EditReplace), SIGNAL(triggered()), this, SLOT(slotReplace()));
	connect(st->action(Juff::EditGotoLine), SIGNAL(triggered()), this, SLOT(slotGotoLine()));
	
	connect(st->action(Juff::ViewLineNumbers), SIGNAL(triggered()), this, SLOT(slotShowLineNumbers()));
	connect(st->action(Juff::ViewWrapWords), SIGNAL(triggered()), this, SLOT(slotWrapWords()));
	connect(st->action(Juff::ViewWhitespaces), SIGNAL(triggered()), this, SLOT(slotShowWhitespaces()));
	connect(st->action(Juff::ViewLineEndings), SIGNAL(triggered()), this, SLOT(slotShowLineEndings()));
	connect(st->action(Juff::ViewZoomIn), SIGNAL(triggered()), this, SLOT(slotZoomIn()));
	connect(st->action(Juff::ViewZoomOut), SIGNAL(triggered()), this, SLOT(slotZoomOut()));
	connect(st->action(Juff::ViewZoom100), SIGNAL(triggered()), this, SLOT(slotZoom100()));
	connect(st->action(Juff::ViewFullscreen), SIGNAL(triggered()), this, SLOT(slotFullscreen()));
	
	connect(st->action(Juff::About), SIGNAL(triggered()), mw_, SLOT(about()));
	connect(st->action(Juff::AboutQt), SIGNAL(triggered()), mw_, SLOT(aboutQt()));

	// toolbar
	QToolBar* tb = mw_->addToolBar("Main");
	int sz = IconManager::instance()->iconSize();
	tb->setIconSize(QSize(sz, sz));
	tb->addAction(st->action(Juff::FileNew));
	tb->addAction(st->action(Juff::FileOpen));
	tb->addAction(st->action(Juff::FileSave));
	tb->addSeparator();
	tb->addAction(st->action(Juff::FilePrint));
	tb->addSeparator();
	tb->addAction(st->action(Juff::EditUndo));
	tb->addAction(st->action(Juff::EditRedo));
	tb->addSeparator();
	tb->addAction(st->action(Juff::EditCut));
	tb->addAction(st->action(Juff::EditCopy));
	tb->addAction(st->action(Juff::EditPaste));
//	tb->addSeparator();
//	tb->addAction(st->action(Juff::EditFind));
//	tb->addAction(st->action(Juff::EditReplace));
	
	
	// Menu //////
	
	QMenu* fileMenu = *( menus_.insert(Juff::MenuFile, new QMenu(tr("&File"))) );
	prjMenu_ = new QMenu(tr("Project"));
	fileMenu->addMenu(prjMenu_);
	fileMenu->addSeparator();
	{
		Juff::ActionID ids[] = { Juff::FileNew, Juff::FileOpen, Juff::FileSave, 
		                         Juff::FileSaveAs, Juff::FileSaveAll, Juff::FileReload, Juff::FileRename,
		                         Juff::Separator, Juff::FileClose, Juff::FileCloseAll,
		                         Juff::FilePrint,
		                         Juff::Separator, Juff::FileExit, Juff::NullID };
		for (int i = 0; ids[i] != Juff::NullID; i++) {
			if ( ids[i] == Juff::Separator )
				fileMenu->addSeparator();
			else
				fileMenu->addAction(st->action(ids[i]));
		}
	}
	
	// project
	{
		Juff::ActionID ids[] = { Juff::PrjNew, Juff::PrjOpen, Juff::PrjSaveAs,
		                         Juff::PrjClose, Juff::PrjAddFile, Juff::NullID };
		for (int i = 0; ids[i] != Juff::NullID; i++) {
			if ( ids[i] == Juff::Separator )
				prjMenu_->addSeparator();
			else
				prjMenu_->addAction(st->action(ids[i]));
		}
	}
	
	QMenu* editMenu = *( menus_.insert(Juff::MenuEdit, new QMenu(tr("&Edit"))) );
	{
		Juff::ActionID ids[] = { Juff::EditUndo, Juff::EditRedo, Juff::Separator,
		                         Juff::EditCut, Juff::EditCopy, Juff::EditPaste,
		                         Juff::Separator,
//			                     Juff::EditFind, Juff::EditFindNext,
//		                         Juff::EditFindPrev, Juff::EditReplace, Juff::Separator,
		                         Juff::EditGotoLine,
		                         Juff::NullID };
		for (int i = 0; ids[i] != Juff::NullID; i++) {
			if ( ids[i] == Juff::Separator )
				editMenu->addSeparator();
			else
				editMenu->addAction(st->action(ids[i]));
		}
	}
	
	QMenu* viewMenu = *( menus_.insert(Juff::MenuView, new QMenu(tr("&View"))) );
	{
		Juff::ActionID ids[] = { Juff::ViewWrapWords, Juff::ViewLineNumbers, Juff::ViewWhitespaces,
		                         Juff::ViewLineEndings,
		                         Juff::Separator,
		                         Juff::ViewZoomIn, Juff::ViewZoomOut, Juff::ViewZoom100, 
		                         Juff::Separator,
		                         Juff::ViewFullscreen,
		                         Juff::NullID };
		for (int i = 0; ids[i] != Juff::NullID; i++) {
			if ( ids[i] == Juff::Separator )
				viewMenu->addSeparator();
			else
				viewMenu->addAction(st->action(ids[i]));
		}
	}
	
	QMenu* formatMenu = *( menus_.insert(Juff::MenuFormat, new QMenu(tr("&Format"))) );
	QMenu* helpMenu = *( menus_.insert(Juff::MenuHelp, new QMenu(tr("&Help"))) );
	helpMenu->addAction(st->action(Juff::About));
	helpMenu->addAction(st->action(Juff::AboutQt));
	
	mw_->menuBar()->addMenu(fileMenu);
	mw_->menuBar()->addMenu(editMenu);
	mw_->menuBar()->addMenu(viewMenu);
	mw_->menuBar()->addMenu(formatMenu);
	mw_->menuBar()->addMenu(helpMenu);
	
	initPlugins();
	
	openWithCharsetGr_ = new QActionGroup(this);
	setCharsetGr_ = new QActionGroup(this);
	charsetMenu_ = new QMenu(tr("Charset"));
	openWithCharsetMenu_ = new QMenu(tr("Open with..."));
	setCharsetMenu_ = new QMenu(tr("Set charset"));
	initCharsetMenus();
	charsetMenu_->addMenu(openWithCharsetMenu_);
	charsetMenu_->addMenu(setCharsetMenu_);
	formatMenu->addMenu(charsetMenu_);
	
	viewer_ = new DocViewer();
//	mw_->setCentralWidget(viewer_);
	mw_->setViewer(viewer_);
	
	docManager_ = new DocManager(this);
	
	connect(viewer_, SIGNAL(docActivated(Juff::Document*)), SLOT(onDocActivated(Juff::Document*)));
	connect(mw_, SIGNAL(closeRequested(bool&)), SLOT(onCloseRequested(bool&)));
	
	// engines actions
	docManager_->initMenuActions(Juff::MenuEdit, editMenu);
	docManager_->initMenuActions(Juff::MenuView, viewMenu);
	
	
	// statusbar
	posL_ = new Juff::StatusLabel("");
	nameL_ = new Juff::StatusLabel("");
	charsetL_ = new Juff::StatusLabel("");
	linesL_ = new Juff::StatusLabel("");
	posL_->setMinimumWidth(130);
	posL_->setToolTip(QObject::tr("Cursor position"));
	nameL_->setToolTip(QObject::tr("File full name"));
	charsetL_->setToolTip(QObject::tr("Current character set"));
	linesL_->setToolTip(QObject::tr("Lines count"));
	charsetL_->setMenu(charsetMenu_);
	connect(linesL_, SIGNAL(clicked()), SLOT(slotGotoLine()));
	connect(posL_, SIGNAL(clicked()), SLOT(slotGotoLine()));
	
	mw_->addStatusWidget(posL_);
	mw_->addStatusWidget(nameL_);
	mw_->addStatusWidget(linesL_);
	mw_->addStatusWidget(charsetL_);

	posL_->hide();
	nameL_->hide();
	charsetL_->hide();
	linesL_->hide();
	
	docManager_->initStatusBar(mw_->statusBar());
	
	loadProject();
}

JuffEd::~JuffEd() {
	Settings::write();
	
	if ( prj_ != NULL )
		delete prj_;
}

QWidget* JuffEd::mainWindow() const {
	return mw_;
}

void JuffEd::slotFileNew() {
	LOGGER;
	
	createDoc("");
}

void JuffEd::slotFileOpen() {
	LOGGER;
	
	// TODO : proper filters here
	QString filters = "All files (*)";
	QStringList files = mw_->getOpenFileNames(openDialogDirectory(), filters);
	
	QString fileName;
	foreach (fileName, files) {
		createDoc(fileName);
	}
	
	// store the last used directory
	MainSettings::set(MainSettings::LastDir, QFileInfo(fileName).absolutePath());
}

void JuffEd::slotFileSave() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( doc->isNull() )
		return;
	
	if ( Juff::isNoname(doc) ) {
		slotFileSaveAs();
	}
	else {
		saveDoc(doc);
	}
}

void JuffEd::slotFileSaveAs() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( doc->isNull() )
		return;
	
	saveDocAs(doc);
}

void JuffEd::slotFileClose() {
	LOGGER;

	Juff::Document* doc = curDoc();
	if ( doc->isNull() )
		return;
	
	closeDocWithConfirmation(doc);
}

void JuffEd::slotFileCloseAll() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	while ( !doc->isNull() ) {
		if ( !closeDocWithConfirmation(doc) )
			break;
		doc = curDoc();
	}
}

void JuffEd::slotFileSaveAll() {
	LOGGER;
	
	QList<Juff::Document*> docs = viewer_->docList();
	foreach (Juff::Document* doc, docs) {
		if ( Juff::isNoname(doc) )
			saveDocAs(doc);
		else
			saveDoc(doc);
	}
}

void JuffEd::slotFileReload() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( doc->isNull() )
		return;
	
	doc->reload();
}

void JuffEd::slotFileRename() {
	LOGGER;
	
}

void JuffEd::slotFilePrint() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( doc->isNull() )
		return;
	
	doc->print();
}

void JuffEd::slotFileExit() {
	LOGGER;
	
	bool exit;
	onCloseRequested(exit);
	
	if ( exit ) {
		qApp->quit();
	}
}

////////////////////////////////////////
// Project

void JuffEd::slotPrjNew() {
	LOGGER;
	
	QString prjFile = mw_->getSavePrjName(tr("New project"));
	if ( !prjFile.isEmpty() ) {
		// store the last used directory
		MainSettings::set(MainSettings::LastDir, QFileInfo(prjFile).absolutePath());
		
		if ( closeProject() )
			createProject(prjFile);
	}
}

void JuffEd::slotPrjOpen() {
	LOGGER;
	
	QString prjFile = mw_->getOpenFileName(openDialogDirectory(), "JuffEd XML Project Files (*.xml)");
	if ( !prjFile.isEmpty() ) {
		// store the last used directory
		MainSettings::set(MainSettings::LastDir, QFileInfo(prjFile).absolutePath());
		
		if ( closeProject() )
			createProject(prjFile);
	}
}

void JuffEd::slotPrjClose() {
	LOGGER;

	if ( closeProject() )
		createProject("");
}

void JuffEd::slotPrjRename() {
	LOGGER;
}

void JuffEd::slotPrjSaveAs() {
	LOGGER;
	
	QString prjName = mw_->getSavePrjName(tr("Save project as..."));
	if ( !prjName.isEmpty() ) {
	}
}

void JuffEd::slotPrjAddFile() {
	LOGGER;
	
	if ( prj_ != NULL ) {
		// TODO : filters
		QStringList fileList = mw_->getOpenFileNames(openDialogDirectory(), "All files (*)");
		QString file;
		foreach (file, fileList) {
			prj_->addFile(file);
		}
		// store the last used directory
		MainSettings::set(MainSettings::LastDir, QFileInfo(file).absolutePath());
	}
}

////////////////////////////////////////
// Edit

void JuffEd::slotEditUndo() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->undo();
	}
}

void JuffEd::slotEditRedo() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->redo();
	}
}

void JuffEd::slotEditCut() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->cut();
	}
}

void JuffEd::slotEditCopy() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->copy();
	}
}

void JuffEd::slotEditPaste() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->paste();
	}
}


////////////////////////////////////////////////////////////////////////////////
// Find

void JuffEd::slotFind() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
	}
}

void JuffEd::slotFindNext() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
	}
}

void JuffEd::slotFindPrev() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
	}
}

void JuffEd::slotReplace() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
	}
}

void JuffEd::slotGotoLine() {
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		int line = mw_->getGotoLineNumber(doc->lineCount());
		if ( line >= 0 )
			doc->gotoLine(line);
	}
}

void JuffEd::slotWrapWords(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->setWrapWords(CommandStorage::instance()->action(Juff::ViewWrapWords)->isChecked());
	}
}

void JuffEd::slotShowLineNumbers(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->setShowLineNumbers(CommandStorage::instance()->action(Juff::ViewLineNumbers)->isChecked());
	}
}

void JuffEd::slotShowWhitespaces(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->setShowWhitespaces(CommandStorage::instance()->action(Juff::ViewWhitespaces)->isChecked());
	}
}

void JuffEd::slotShowLineEndings(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->setShowLineEndings(CommandStorage::instance()->action(Juff::ViewLineEndings)->isChecked());
	}
}

void JuffEd::slotZoomIn(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->zoomIn();
	}
}

void JuffEd::slotZoomOut(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->zoomOut();
	}
}

void JuffEd::slotZoom100(){
	LOGGER;
	
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() ) {
		doc->zoom100();
	}
}

void JuffEd::slotFullscreen() {
	LOGGER;
	
	mw_->setWindowState(mw_->windowState() ^ Qt::WindowFullScreen);
}

void JuffEd::slotOpenWithCharset() {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(curDoc());
	QAction* action = qobject_cast<QAction*>(sender());
	if ( doc != 0 && action != 0 ) {
		QString charset = doc->charset();
		doc->setCharset(action->text());
		doc->reload();
	}
}

void JuffEd::slotSetCharset() {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(curDoc());
	QAction* action = qobject_cast<QAction*>(sender());
	if ( doc != 0 && action != 0 ) {
		QString charset = doc->charset();
		doc->setCharset(action->text());
	}
}



////////////////////////////////////////////////////////////////////////////////
// Slots

void JuffEd::onDocModified(bool) {
	LOGGER;

	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		updateGUI(doc);
		
		// notify plugins
		emit docModified(doc);
	}
}

void JuffEd::onDocCursorMoved(int line, int col) {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		updateCursorPos(doc);
	}
}

void JuffEd::onDocTextChanged() {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		// notify plugins
		emit docTextChanged(doc);
	}
}

void JuffEd::onDocLineCountChanged(int lines) {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		updateLineCount(doc);
	}
}

void JuffEd::onDocActivated(Juff::Document* doc) {
	LOGGER;

	updateGUI(doc);
	updateMenus(doc);
	
	// notify plugins
	emit docActivated(doc);
}

void JuffEd::onCloseRequested(bool& confirm) {
	LOGGER;

	QMap <QString, Juff::Document*> unsaved;
	QList<Juff::Document*> docs = viewer_->docList();
	foreach (Juff::Document* doc, docs)
		if ( doc->isModified() )
			unsaved[doc->fileName()] = doc;
	
	confirm = true;
	if ( !unsaved.isEmpty() ) {
		QStringList toSave;
		
		// ask for list of files needed to save
		if ( mw_->askForSave(unsaved.keys(), toSave) ) {
			foreach (QString fileName, toSave) {
				Juff::Document* doc = unsaved.value(fileName, 0);
				if ( 0 != doc ) {
					if ( Juff::isNoname(fileName) ) {
						if ( !saveDocAs(doc) ) {
							// saving failed or was cancelled
							confirm = false;
						}
					}
					else {
						if ( !saveDoc(doc) ) {
							// saving failed
							confirm = false;
						}
					}
				}
			}
		}
		else {
			// closing was cancelled
			confirm = false;
		}
	}
}


Juff::Document* JuffEd::createDoc(const QString& fileName) {
	LOGGER;
	
	Juff::Document* doc;
	if ( fileName.isEmpty() )
		doc = docManager_->newDoc();
	else
		doc = docManager_->openDoc(fileName);
	
	if ( !doc->isNull() ) {
		initDoc(doc);
		viewer_->addDoc(doc);
		if ( prj_->name().isEmpty() )
			prj_->addFile(doc->fileName());
		updateGUI(doc);
	}
	
	// notify plugins
	emit docOpened(doc);
	
	return doc;
}

void JuffEd::createProject(const QString& fileName) {
	prj_ = new Juff::Project(fileName);
	MainSettings::set(MainSettings::LastProject, prj_->fileName());
	loadProject();
	
	// TODO : remove later
	tree_->setProject(prj_);
}

bool JuffEd::closeProject() {
	if ( prj_ == NULL )
		return true;
	
	slotFileCloseAll();
	// check if all files were closed
	if ( viewer_->docCount() > 0 )
		return false;
	
	delete prj_;
	return true;
}


void JuffEd::initDoc(Juff::Document* doc) {
	LOGGER;
	
	connect(doc, SIGNAL(modified(bool)), SLOT(onDocModified(bool)));
	connect(doc, SIGNAL(cursorPosChanged(int, int)), SLOT(onDocCursorMoved(int, int)));
	connect(doc, SIGNAL(lineCountChanged(int)), SLOT(onDocLineCountChanged(int)));
	connect(doc, SIGNAL(textChanged()), SLOT(onDocTextChanged()));
	
	updateMenus(doc);
}


////////////////////////////////////////////////////////////////////////////////
// Implementation of DocHandletInt interface
Juff::Document* JuffEd::curDoc() const {
	return viewer_->currentDoc();
}

void JuffEd::openDoc(const QString& fileName) {
	LOGGER;
	
	if ( !viewer_->activateDoc(fileName) )
		createDoc(fileName);
}

void JuffEd::closeDoc(const QString& fileName) {
	LOGGER;
	
	Juff::Document* doc = viewer_->document(fileName);
	if ( !doc->isNull() )
		closeDocWithConfirmation(doc);
}

void JuffEd::saveDoc(const QString& fileName) {
	LOGGER;
	
	Juff::Document* doc = viewer_->document(fileName);
	if ( !doc->isNull() )
		saveDoc(doc);
}

int JuffEd::docCount() const {
	LOGGER;
	return viewer_->docCount();
}

QStringList JuffEd::docList() const {
	LOGGER;
	return viewer_->docNamesList();
}




void JuffEd::reportError(const QString& error) {
	QMessageBox::warning(mw_, tr("Error"), error);
}

bool JuffEd::closeDocWithConfirmation(Juff::Document* doc) {
	if ( doc == 0 || doc->isNull() )
		return true;
	
	bool decidedToClose = true;
	
	if ( doc->isModified() ) {
		switch (mw_->askForSave(doc->fileName())) {
			case QMessageBox::Save :
			{
				bool confirmed = Juff::isNoname(doc) ? saveDocAs(doc) : saveDoc(doc);
				decidedToClose = confirmed;
				break;
			}
			
			case QMessageBox::Cancel :
				decidedToClose = false;
				break;
			
			case QMessageBox::Discard :
			default:;
		}
	}

	if ( decidedToClose ) {
		if ( prj_ != NULL && prj_->isNoname() )
			prj_->removeFile(doc->fileName());
		
		// notify plugins
		emit docClosed(doc);
		
		delete doc;
		return true;
	}
	else {
		return false;
	}
}

bool JuffEd::saveDoc(Juff::Document* doc) {
	if ( doc->isModified() ) {
		QString error;
		if ( doc->save(error) ) {
			// saving succeeded - return true
			return true;
		}
		else {
			// saving failed or was interrupted - check for error and return false
			if ( !error.isEmpty() )
				reportError(error);
			return false;
		}
	}
	return true;
}

bool JuffEd::saveDocAs(Juff::Document* doc) {
	QString filters = "All files (*)";
	QString fileName = mw_->getSaveFileName(doc->fileName(), filters);
	if ( !fileName.isEmpty() ) {
		// store the last used directory
		MainSettings::set(MainSettings::LastDir, QFileInfo(fileName).absolutePath());
		
		QString error;
		if ( doc->saveAs(fileName, error) ) {
			// saving succeeded - return true
			return true;
		}
		else {
			// saving failed or was interrupted - check for error and return false
			if ( !error.isEmpty() )
				reportError(error);
			return false;
		}
	}
	else {
		// file selecting dialog was cancelled
		return false;
	}
}

void JuffEd::updateMenus(Juff::Document* doc) {
	LOGGER;
	
	CommandStorage::instance()->action(Juff::ViewWrapWords)->setChecked(doc->wrapWords());
	CommandStorage::instance()->action(Juff::ViewLineNumbers)->setChecked(doc->lineNumbersVisible());
	CommandStorage::instance()->action(Juff::ViewWhitespaces)->setChecked(doc->whitespacesVisible());
	CommandStorage::instance()->action(Juff::ViewLineEndings)->setChecked(doc->lineEndingsVisible());

	docManager_->setCurDocType(doc->type());
}

void JuffEd::updateGUI(Juff::Document* doc) {
	QString title;
	if ( !doc->isNull() ) {
		title = QString("%1 - ").arg(Juff::docTitle(doc));
		if ( !projectName().isEmpty() )
			title += QString("[%1] - ").arg(projectName());
		
		posL_->show();
		nameL_->show();
		charsetL_->show();
		linesL_->show();
		
		nameL_->setText(doc->fileName());
		charsetL_->setText(doc->charset());
		
		updateLineCount(doc);
		updateCursorPos(doc);
	}
	else {
		posL_->hide();
		nameL_->hide();
		charsetL_->hide();
		linesL_->hide();
	}
	
	title += "JuffEd";
	mw_->setWindowTitle(title);
}

void JuffEd::updateLineCount(Juff::Document* doc) {
	linesL_->setText(tr("Lines: %1").arg(doc->lineCount()));
}

void JuffEd::updateCursorPos(Juff::Document* doc) {
	int line, col;
	doc->getCursorPos(line, col);
	posL_->setText(tr("Line: %1, Col: %2").arg(line+1).arg(col+1));
}



void JuffEd::initCharsetMenus() {
	LOGGER;

	openWithCharsetMenu_->clear();
	setCharsetMenu_->clear();
	
	
	foreach (QAction* a, openWithCharsetGr_->actions())
		openWithCharsetGr_->removeAction(a);

	QStringList charsets = CharsetSettings::getCharsetsList();
	foreach (QString charset, charsets) {
		if ( CharsetSettings::charsetEnabled(charset) ) {
			QAction* openAct = openWithCharsetMenu_->addAction(charset, this, SLOT(slotOpenWithCharset()));
			openAct->setCheckable(true);
			openWithCharsetGr_->addAction(openAct);
			
			QAction* setAct = setCharsetMenu_->addAction(charset, this, SLOT(slotSetCharset()));
			setAct->setCheckable(true);
			setCharsetGr_->addAction(setAct);
			
//			mInt_->charsetActions_[charset] = action;
		}
	}
}

void JuffEd::initPlugins() {
	pluginMgr_.loadPlugins();
	
	Juff::MenuList menus = pluginMgr_.menus();
	foreach (QMenu* menu, menus)
		mw_->menuBar()->addMenu(menu);
}

QString JuffEd::projectName() const {
	return ( prj_ == NULL ? "" : prj_->name() );
}

void JuffEd::loadProject() {
	if ( prj_ == NULL ) return;
	
	QStringList files = prj_->files();
	foreach (QString file, files) {
		openDoc(file);
	}
}

QString JuffEd::openDialogDirectory() const {
	Juff::Document* doc = curDoc();
	if ( !doc->isNull() && !Juff::isNoname(doc) && MainSettings::get(MainSettings::SyncToCurDoc) )
		return QFileInfo(doc->fileName()).absolutePath();
	else
		return MainSettings::get(MainSettings::LastDir);
}