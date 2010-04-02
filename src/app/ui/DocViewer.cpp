#include <QDebug>

#include "DocViewer.h"

#include "Document.h"
#include "Functions.h"
#include "Log.h"
#include "NullDoc.h"
#include "TabWidget.h"

#include <QAction>
#include <QMenu>
//#include <QTabBar>
#include <QVBoxLayout>

DocViewer::DocViewer() : QWidget() {
	spl_ = new QSplitter(this);
	QVBoxLayout* vBox = new QVBoxLayout(this);
	vBox->addWidget(spl_);
	vBox->setContentsMargins(0, 0, 0, 0);
	setLayout(vBox);

	tab1_ = new Juff::TabWidget();
	tab2_ = new Juff::TabWidget();
	spl_->addWidget(tab1_);
	spl_->addWidget(tab2_);
	curTab_ = tab1_;
	
	spl_->setSizes(QList<int>() << spl_->width() << 0);
	
	Juff::TabWidget* tabWidgets[] = { tab1_, tab2_, NULL };
	for (int i = 0; tabWidgets[i] != NULL; ++i) {
		Juff::TabWidget* tw = tabWidgets[i];
		
		connect(tw, SIGNAL(requestDocClose(Juff::Document*, Juff::TabWidget*)), SLOT(onDocCloseRequested(Juff::Document*, Juff::TabWidget*)));
		connect(tw, SIGNAL(requestDocClone(Juff::Document*, Juff::TabWidget*)), SLOT(onDocCloneRequested(Juff::Document*, Juff::TabWidget*)));
		connect(tw, SIGNAL(requestDocMove(Juff::Document*, Juff::TabWidget*)), SLOT(onDocMoveRequested(Juff::Document*, Juff::TabWidget*)));
		connect(tw, SIGNAL(tabRemoved(Juff::TabWidget*)), SLOT(onTabRemoved(Juff::TabWidget*)));
	}
	
	// next/prev doc
	nextAct_ = new QAction("Next", this);
	prevAct_ = new QAction("Prev", this);
	nextAct_->setShortcut(QKeySequence("Ctrl+PgDown"));
	prevAct_->setShortcut(QKeySequence("Ctrl+PgUp"));
	connect(nextAct_, SIGNAL(triggered()), SLOT(nextDoc()));
	connect(prevAct_, SIGNAL(triggered()), SLOT(prevDoc()));
	addAction(nextAct_);
	addAction(prevAct_);
	
	// numbered docs
	for (int i = 0; i < 10; ++i) {
		QAction* act = new QAction(QString::number(i), this);
		act->setShortcut(QKeySequence(QString("Alt+%1").arg(i)));
		connect(act, SIGNAL(triggered()), SLOT(goToNumberedDoc()));
		addAction(act);
	}
	curDoc_ = NullDoc::instance();
}

void DocViewer::addDoc(Juff::Document* doc) {
	LOGGER;
	
	connect(doc, SIGNAL(modified(bool)), SLOT(onDocModified(bool)));

	addDoc(doc, curTab_);
	
	// It's better to have it after adding to TabWidget to avoid
	// emitting the signal 'docActivated()' during the document creation.
	connect(doc, SIGNAL(focused()), SLOT(onDocFocused()));
}

Juff::Document* DocViewer::currentDoc() const {
	Juff::Document* doc = qobject_cast<Juff::Document*>(curTab_->currentWidget());
	if ( doc != 0 )
		return doc;
	else
		return NullDoc::instance();
}

Juff::Document* DocViewer::document(const QString& fileName) const {
	QList<Juff::Document*> docs = docList(1);
	foreach (Juff::Document* doc, docs) {
		if ( doc->fileName() == fileName ) {
			return doc;
		}
	}
	
	docs = docList(2);
	foreach (Juff::Document* doc, docs) {
		if ( doc->fileName() == fileName ) {
			return doc;
		}
	}
	
	return NullDoc::instance();
}

bool DocViewer::activateDoc(const QString& fileName) {
	QList<Juff::Document*> docs = docList(1);
	foreach (Juff::Document* doc, docs) {
		if ( doc->fileName() == fileName ) {
			tab1_->setCurrentWidget(doc);
			doc->setFocus();
			return true;
		}
	}
	
	docs = docList(2);
	foreach (Juff::Document* doc, docs) {
		if ( doc->fileName() == fileName ) {
			tab2_->setCurrentWidget(doc);
			doc->setFocus();
			return true;
		}
	}
	
	return false;
}

int DocViewer::docCount(int panel) const {
	switch (panel) {
		case 0 :
			return tab1_->count() + tab2_->count();
		case 1 :
			return tab1_->count();
		case 2 :
			return tab2_->count();
		default: return 0;
	}
}

QList<Juff::Document*> DocViewer::docList(int panel) const {
	QList<Juff::Document*> list;
	
	// 1st panel
	if ( panel == 0 || panel == 1 ) {
		int n = tab1_->count();
		for (int i = 0; i < n; ++i) {
			Juff::Document* doc = qobject_cast<Juff::Document*>(tab1_->widget(i));
			if ( doc != 0 )
				list << doc;
		}
	}
	// 2nd panel
	if ( panel == 0 || panel == 2 ) {
		int n = tab2_->count();
		for (int i = 0; i < n; ++i) {
			Juff::Document* doc = qobject_cast<Juff::Document*>(tab2_->widget(i));
			if ( doc != 0 )
				list << doc;
		}
	}
	return list;
}

QStringList DocViewer::docNamesList(int panel) const {
	QStringList list;
	// 1st panel
	if ( panel == 0 || panel == 1 ) {
		int n = tab1_->count();
		for (int i = 0; i < n; ++i) {
			Juff::Document* doc = qobject_cast<Juff::Document*>(tab1_->widget(i));
			if ( doc != 0 )
				list << doc->fileName();
		}
	}
	// 2nd panel
	if ( panel == 0 || panel == 2 ) {
		int n = tab2_->count();
		for (int i = 0; i < n; ++i) {
			Juff::Document* doc = qobject_cast<Juff::Document*>(tab2_->widget(i));
			if ( doc != 0 )
				list << doc->fileName();
		}
	}
	return list;
}

void DocViewer::nextDoc() {
	LOGGER;
	
	int n = curTab_->count();
	if ( n == 0 )
		return;
	
	curTab_->setCurrentIndex( (curTab_->currentIndex() + 1) % n );
}

void DocViewer::prevDoc() {
	LOGGER;
	
	int n = curTab_->count();
	if ( n == 0 )
		return;
	
	curTab_->setCurrentIndex( (curTab_->currentIndex() + n - 1) % n );
}

void DocViewer::goToNumberedDoc() {
	LOGGER;
	
	QAction* act = qobject_cast<QAction*>(sender());
	if ( act == 0 )
		return;
	
	int index = act->text().toInt();
	if ( index == 0 )
		index = 10;
	curTab_->setCurrentIndex(index - 1);
}





void DocViewer::onDocModified(bool modified) {
	LOGGER;

	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		int index = tab1_->indexOf(doc);
		if ( index >= 0 ) {
			tab1_->setTabText(index, Juff::docTitle(doc->fileName(), doc->isModified()));
			if ( doc->isModified() )
//				tab1_->setTabIcon(index, QIcon(":doc_modified"));
				tab1_->setTabIcon(index, QIcon(":doc_icon_red"));
			else
//				tab1_->setTabIcon(index, QIcon(":doc_unmodified"));
				tab1_->setTabIcon(index, QIcon(":doc_icon"));
		}
		else {
			index = tab2_->indexOf(doc);
			if ( index >= 0 ) {
				tab2_->setTabText(index, Juff::docTitle(doc->fileName(), doc->isModified()));
				if ( doc->isModified() )
//					tab2_->setTabIcon(index, QIcon(":doc_modified"));
					tab2_->setTabIcon(index, QIcon(":doc_icon_red"));
				else
//					tab2_->setTabIcon(index, QIcon(":doc_unmodified"));
					tab2_->setTabIcon(index, QIcon(":doc_icon"));
			}
			Log::warning("Document that emitted a signal is not owned by DocViewer");
		}
	}
}

void DocViewer::onDocCloseRequested(Juff::Document* doc, Juff::TabWidget* tw) {
	LOGGER;
	
	int oldIndex = tw->indexOf(doc);
	if ( oldIndex < 0 )
		return;
	
	doc->deleteLater();
	
	if ( tw->count() == 0 ) {
//		tw->hide();
		Juff::TabWidget* tw2 = (tw == tab1_ ? tab2_ : tab1_);
		if ( tw2->count() == 0 ) {
			tab2_->hide();
			curTab_ = tab1_;
		}
		else {
			tab1_->hide();
			curTab_ = tab2_;
		}
	}
	else {
		if ( tw->currentIndex() == oldIndex ) {
			Juff::Document* newCurDoc = qobject_cast<Juff::Document*>(tw->currentWidget());
			if ( newCurDoc != 0 ) {
				curTab_ = tw;
//				emit docActivated(newCurDoc);
			}
		}
	}
}

void DocViewer::onDocCloneRequested(Juff::Document*, Juff::TabWidget* tw) {
	LOGGER;
}

void DocViewer::onDocMoveRequested(Juff::Document* doc, Juff::TabWidget* tw) {
	LOGGER;

	Juff::TabWidget* tw2 = anotherPanel(tw);
	curTab_ = tw2;
	addDoc(doc, tw2);
	if ( tw2->width() == 0 ) {
		spl_->setSizes(QList<int>() << spl_->width() / 2 << spl_->width() / 2);
	}
	if ( tw->count() == 0 ) {
		closePanel(tw);
	}
}

void DocViewer::onTabRemoved(Juff::TabWidget* tw) {
	LOGGER;
	
	Juff::TabWidget* tw2 = anotherPanel(tw);
	if ( tw->count() == 0 ) {
		if ( tw2->count() > 0 ) {
			closePanel(tw);
			tw2->currentWidget()->setFocus();
		}
		else {
			emit docActivated(NullDoc::instance());
		}
	}
	else {
		tw->currentWidget()->setFocus();
	}
}

void DocViewer::onDocFocused() {
	LOGGER;
	
	Juff::Document* doc = qobject_cast<Juff::Document*>(sender());
	if ( doc != 0 ) {
		if ( doc != curDoc_ ) {
			curDoc_ = doc;
			emit docActivated(doc);
		}
		
		if ( tab1_->indexOf(doc) >= 0 )
			curTab_ = tab1_;
		else
			curTab_ = tab2_;
	}
}


Juff::TabWidget* DocViewer::anotherPanel(Juff::TabWidget* tw) {
	return (tw == tab1_ ? tab2_ : tab1_);
}

void DocViewer::addDoc(Juff::Document* doc, Juff::TabWidget* tabWidget) {
//	QIcon icon(doc->isModified() ? ":doc_modified" : ":doc_unmodified");
	QIcon icon(doc->isModified() ? ":doc_icon_red" : ":doc_icon");
	
	tabWidget->addTab(doc, icon, Juff::docTitle(doc->fileName(), doc->isModified()));
	tabWidget->setCurrentWidget(doc);
	
	doc->init();
	doc->setFocus();
}

void DocViewer::closePanel(Juff::TabWidget* tabWidget) {
	QList<int> list;
	if ( tabWidget == tab1_ ) {
		list << 0 << spl_->width();
		curTab_ = tab2_;
	}
	else {
		list << spl_->width() << 0;
		curTab_ = tab1_;
	}
	spl_->setSizes(list);
}