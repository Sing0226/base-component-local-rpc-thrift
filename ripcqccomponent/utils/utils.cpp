#include "stdafx.h"
#include "utils.h"

namespace KWpsDspsIpcUtils
{
	void writeLog(const QString& msg)
	{
		if (msg.isEmpty())
			return;

		static QString logFilePath = QString("%1/wpsdspsipc_%2.log").arg(QDir::tempPath()).arg(QCoreApplication::applicationName());

		QFile file(logFilePath);
		file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);

		QTextStream out(&file);
		QString datetimeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
		out << datetimeStr << " " << msg << "\n";
	}
}
