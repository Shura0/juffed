/*
JuffEd - An advanced text editor
Copyright 2007-2010 Mikhail Murzin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License 
version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef __JUFF_STATUS_LABEL_H__
#define __JUFF_STATUS_LABEL_H__

class QMenu;

#include <QtGui/QLabel>
#include "LibConfig.h"

namespace Juff {

class LIBJUFF_EXPORT StatusLabel : public QLabel {
Q_OBJECT
public:
	StatusLabel(const QString& text);
	void setMenu(QMenu* menu);

signals:
	void clicked();

protected:
	virtual void mouseReleaseEvent(QMouseEvent*);
	
private:
	QMenu* menu_;
};


};	//	namespace Juff

#endif // __JUFF_STATUS_LABEL_H__
