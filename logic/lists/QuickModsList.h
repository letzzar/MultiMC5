#pragma once

#include <QAbstractListModel>
#include <QUrl>

class QuickModFilesUpdater;

class QuickMod : public QObject
{
	Q_OBJECT
public:
	QuickMod(QObject *parent = 0);

	enum ModType
	{
		ForgeMod,
		ForgeCoreMod,
		ResourcePack,
		ConfigPack
	};
	Q_ENUMS(ModType)

	struct Version
	{
		QString name;
		QUrl url;
		QStringList compatibleVersions;
	};

	bool isValid() const
	{
		return !m_name.isEmpty();
	}

	QString name() const { return m_name; }
	QString description() const { return m_description; }
	QUrl websiteUrl() const { return m_websiteUrl; }
	QUrl iconUrl() const { return m_iconUrl; }
	QUrl updateUrl() const { return m_updateUrl; }
	QList<QUrl> recommendedUrls() const { return m_recommendedUrls; }
	QList<QUrl> dependentUrls() const { return m_dependentUrls; }
	QString nemName() const { return m_nemName; }
	QString modId() const { return m_modId; }
	QStringList categories() const { return m_categories; }
	QStringList tags() const { return m_tags; }
	ModType type() const { return m_type; }
	QList<Version> versions() const { return m_versions; }

	int numVersions() const { return m_versions.size(); }
	Version version(const int index) const { return m_versions.at(index); }

	bool parse(const QByteArray &data, QString *errorMessage = 0);

	bool compare(const QuickMod *other) const;

private:
	QString m_name;
	QString m_description;
	QUrl m_websiteUrl;
	QUrl m_iconUrl;
	QUrl m_updateUrl;
	QList<QUrl> m_recommendedUrls;
	QList<QUrl> m_dependentUrls;
	QString m_nemName;
	QString m_modId;
	QStringList m_categories;
	QStringList m_tags;
	ModType m_type;
	QList<Version> m_versions;
};
Q_DECLARE_METATYPE(QuickMod*)
Q_DECLARE_METATYPE(QuickMod::Version)

class QuickModsList : public QAbstractListModel
{
	Q_OBJECT
public:
	QuickModsList(QObject *parent = 0);

	enum Roles
	{
		NameRole = Qt::UserRole,
		DescriptionRole,
		WebsiteRole,
		IconRole,
		LogoRole,
		UpdateRole,
		RecommendedUrlsRole,
		DependentUrlsRole,
		NemNameRole,
		ModIdRole,
		CategoriesRole,
		TagsRole,
		TypeRole,
		VersionsRole
	};
	QHash<int, QByteArray> roleNames() const;

	int rowCount(const QModelIndex &) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	QVariant data(const QModelIndex &index, int role) const;

	bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
						 const QModelIndex &parent) const;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
					  const QModelIndex &parent);
	Qt::DropActions supportedDropActions() const;
	Qt::DropActions supportedDragActions() const;

	int numMods() const { return m_mods.size(); }
	QuickMod *modAt(const int index) const { return m_mods[index]; }

public
slots:
	void registerMod(const QString &fileName);
	void registerMod(const QUrl &url);

	void updateFiles();

private
slots:
	void addMod(QuickMod *mod);
	void clearMods();
	void removeMod(QuickMod *mod);

signals:
	void registerModFile(const QUrl &url);
	void updateModFiles();

private:
	QuickModFilesUpdater *m_updater;

	QList<QuickMod *> m_mods;
};