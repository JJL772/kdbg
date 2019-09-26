/*
 * Copyright Johannes Sixt
 * This file is licensed under the GNU General Public License Version 2.
 * See the file COPYING in the toplevel directory of the source directory.
 */

#include "watchwindow.h"
#include <klocalizedstring.h>		/* i18n */
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QMimeData>
#include <QMenu>
#include <QSignalMapper>

WatchWindow::WatchWindow(QWidget* parent) :
	QWidget(parent),
	m_watchEdit(this),
	m_watchAdd(i18n(" Add "), this),
	m_watchDelete(i18n(" Del "), this),
	m_watchVariables(this, i18n("Expression")),
	m_watchV(this),
	m_watchH()
{
    // setup the layout
    m_watchAdd.setMinimumSize(m_watchAdd.sizeHint());
    m_watchDelete.setMinimumSize(m_watchDelete.sizeHint());
    m_watchV.setMargin(0);
    m_watchV.setSpacing(0);
    m_watchH.setMargin(0);
    m_watchH.setSpacing(0);
    m_watchV.addLayout(&m_watchH);
    m_watchV.addWidget(&m_watchVariables);
    m_watchH.addWidget(&m_watchEdit);
    m_watchH.addWidget(&m_watchAdd);
    m_watchH.addWidget(&m_watchDelete);

	this->setContextMenuPolicy(Qt::CustomContextMenu);
	m_watchVariables.setContextMenuPolicy(Qt::CustomContextMenu);

	connect(&m_watchVariables, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenu(const QPoint&)));
    connect(&m_watchEdit, SIGNAL(returnPressed()), SIGNAL(addWatch()));
    connect(&m_watchAdd, SIGNAL(clicked()), SIGNAL(addWatch()));
    connect(&m_watchDelete, SIGNAL(clicked()), SIGNAL(deleteWatch()));
    connect(&m_watchVariables, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
	    SLOT(slotWatchHighlighted()));

    m_watchVariables.installEventFilter(this);
    setAcceptDrops(true);
}

WatchWindow::~WatchWindow()
{
}

bool WatchWindow::eventFilter(QObject*, QEvent* ev)
{
    if (ev->type() == QEvent::KeyPress)
    {
	QKeyEvent* kev = static_cast<QKeyEvent*>(ev);
	if (kev->key() == Qt::Key_Delete) {
	    emit deleteWatch();
	    return true;
	}
    }
    return false;
}

void WatchWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasText())
	event->acceptProposedAction();
}

void WatchWindow::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasText()) {
	QString text = event->mimeData()->text();
	// pick only the first line
	text = text.trimmed();
	int pos = text.indexOf('\n');
	if (pos > 0)
	    text.truncate(pos);
	text = text.trimmed();
	if (!text.isEmpty())
	    emit textDropped(text);
    }
    event->acceptProposedAction();
}

void WatchWindow::keyPressEvent(QKeyEvent* ev)
{
	if(ev->key() == Qt::Key_Escape && m_watchVariables.selectedItem())
		m_watchVariables.selectedItem()->setSelected(false);
}

void WatchWindow::onCustomContextMenu(const QPoint& pt)
{
	QTreeWidgetItem* item = m_watchVariables.itemAt(pt);
	if(item)
	{
		item->setSelected(true);
		QMenu menu(tr("Action"), this);
		QAction action1("Delete watch", this);
		connect(&action1, &QAction::triggered, this, [this]{
			deleteWatch();
		});	
		QAction action2("Copy watch", this);
		connect(&action2, &QAction::triggered, this, [this]{
			
		});
		menu.addAction(&action1);
		menu.addAction(&action2);
		menu.exec(mapToGlobal(pt));
	}
}

// place the text of the hightlighted watch expr in the edit field
void WatchWindow::slotWatchHighlighted()
{
    VarTree* expr = m_watchVariables.selectedItem();
    QString text = expr ? expr->computeExpr() : QString();
    m_watchEdit.setText(text);
}

#include "watchwindow.moc"
