/* Copyright 2013 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LegacyUpdate.h"
#include "lists/LwjglVersionList.h"
#include "lists/MinecraftVersionList.h"
#include "BaseInstance.h"
#include "LegacyInstance.h"
#include "MultiMC.h"
#include "ModList.h"
#include <pathutils.h>
#include <quazip.h>
#include <quazipfile.h>
#include <JlCompress.h>
#include "logger/QsLog.h"
#include "logic/net/URLConstants.h"
#include <QStringList>

LegacyUpdate::LegacyUpdate(BaseInstance *inst, QObject *parent) : Task(parent), m_inst(inst)
{
	// 1.3 - 1.3.2
	auto libs13 = QList<FMLlib>{
		{"argo-2.25.jar", "bb672829fde76cb163004752b86b0484bd0a7f4b", false},
		{"guava-12.0.1.jar", "b8e78b9af7bf45900e14c6f958486b6ca682195f", false},
		{"asm-all-4.0.jar", "98308890597acb64047f7e896638e0d98753ae82", false}};

	fmlLibsMapping["1.3.2"] = libs13;

	auto libs14 = QList<FMLlib>{
		{"argo-2.25.jar", "bb672829fde76cb163004752b86b0484bd0a7f4b", false},
		{"guava-12.0.1.jar", "b8e78b9af7bf45900e14c6f958486b6ca682195f", false},
		{"asm-all-4.0.jar", "98308890597acb64047f7e896638e0d98753ae82", false},
		{"bcprov-jdk15on-147.jar", "b6f5d9926b0afbde9f4dbe3db88c5247be7794bb", false}};

	fmlLibsMapping["1.4"] = libs14;
	fmlLibsMapping["1.4.1"] = libs14;
	fmlLibsMapping["1.4.2"] = libs14;
	fmlLibsMapping["1.4.3"] = libs14;
	fmlLibsMapping["1.4.4"] = libs14;
	fmlLibsMapping["1.4.5"] = libs14;
	fmlLibsMapping["1.4.6"] = libs14;
	fmlLibsMapping["1.4.7"] = libs14;

	fmlLibsMapping["1.5"] = QList<FMLlib>{
		{"argo-small-3.2.jar", "58912ea2858d168c50781f956fa5b59f0f7c6b51", false},
		{"guava-14.0-rc3.jar", "931ae21fa8014c3ce686aaa621eae565fefb1a6a", false},
		{"asm-all-4.1.jar", "054986e962b88d8660ae4566475658469595ef58", false},
		{"bcprov-jdk15on-148.jar", "960dea7c9181ba0b17e8bab0c06a43f0a5f04e65", true},
		{"deobfuscation_data_1.5.zip", "5f7c142d53776f16304c0bbe10542014abad6af8", false},
		{"scala-library.jar", "458d046151ad179c85429ed7420ffb1eaf6ddf85", true}};

	fmlLibsMapping["1.5.1"] = QList<FMLlib>{
		{"argo-small-3.2.jar", "58912ea2858d168c50781f956fa5b59f0f7c6b51", false},
		{"guava-14.0-rc3.jar", "931ae21fa8014c3ce686aaa621eae565fefb1a6a", false},
		{"asm-all-4.1.jar", "054986e962b88d8660ae4566475658469595ef58", false},
		{"bcprov-jdk15on-148.jar", "960dea7c9181ba0b17e8bab0c06a43f0a5f04e65", true},
		{"deobfuscation_data_1.5.1.zip", "22e221a0d89516c1f721d6cab056a7e37471d0a6", false},
		{"scala-library.jar", "458d046151ad179c85429ed7420ffb1eaf6ddf85", true}};

	fmlLibsMapping["1.5.2"] = QList<FMLlib>{
		{"argo-small-3.2.jar", "58912ea2858d168c50781f956fa5b59f0f7c6b51", false},
		{"guava-14.0-rc3.jar", "931ae21fa8014c3ce686aaa621eae565fefb1a6a", false},
		{"asm-all-4.1.jar", "054986e962b88d8660ae4566475658469595ef58", false},
		{"bcprov-jdk15on-148.jar", "960dea7c9181ba0b17e8bab0c06a43f0a5f04e65", true},
		{"deobfuscation_data_1.5.2.zip", "446e55cd986582c70fcf12cb27bc00114c5adfd9", false},
		{"scala-library.jar", "458d046151ad179c85429ed7420ffb1eaf6ddf85", true}};
}

void LegacyUpdate::executeTask()
{
	/*
	if(m_only_prepare)
	{
		// FIXME: think this through some more.
		LegacyInstance *inst = (LegacyInstance *)m_inst;
		if (!inst->shouldUpdate() || inst->shouldUseCustomBaseJar())
		{
			ModTheJar();
		}
		else
		{
			emitSucceeded();
		}
	}
	else
	{
		*/
	fmllibsStart();
	//}
}

void LegacyUpdate::fmllibsStart()
{
	// Get the mod list
	LegacyInstance *inst = (LegacyInstance *)m_inst;
	auto modList = inst->jarModList();

	bool forge_present = false;

	QString version = inst->intendedVersionId();
	if (!fmlLibsMapping.contains(version))
	{
		lwjglStart();
		return;
	}

	auto &libList = fmlLibsMapping[version];

	// determine if we need some libs for FML or forge
	setStatus(tr("Checking for FML libraries..."));
	for (unsigned i = 0; i < modList->size(); i++)
	{
		auto &mod = modList->operator[](i);

		// do not use disabled mods.
		if (!mod.enabled())
			continue;

		if (mod.type() != Mod::MOD_ZIPFILE)
			continue;

		if (mod.mmc_id().contains("forge", Qt::CaseInsensitive))
		{
			forge_present = true;
			break;
		}
		if (mod.mmc_id().contains("fml", Qt::CaseInsensitive))
		{
			forge_present = true;
			break;
		}
	}
	// we don't...
	if (!forge_present)
	{
		lwjglStart();
		return;
	}

	// now check the lib folder inside the instance for files.
	for (auto &lib : libList)
	{
		QFileInfo libInfo(PathCombine(inst->libDir(), lib.name));
		if (libInfo.exists())
			continue;
		fmlLibsToProcess.append(lib);
	}

	// if everything is in place, there's nothing to do here...
	if (fmlLibsToProcess.isEmpty())
	{
		lwjglStart();
		return;
	}

	// download missing libs to our place
	setStatus(tr("Dowloading FML libraries..."));
	auto dljob = new NetJob("FML libraries");
	auto metacache = MMC->metacache();
	for (auto &lib : fmlLibsToProcess)
	{
		auto entry = metacache->resolveEntry("fmllibs", lib.name);
		QString urlString = lib.ours ? URLConstants::FMLLIBS_OUR_BASE_URL + lib.name
									 : URLConstants::FMLLIBS_FORGE_BASE_URL + lib.name;
		dljob->addNetAction(CacheDownload::make(QUrl(urlString), entry));
	}

	connect(dljob, SIGNAL(succeeded()), SLOT(fmllibsFinished()));
	connect(dljob, SIGNAL(failed()), SLOT(fmllibsFailed()));
	connect(dljob, SIGNAL(progress(qint64, qint64)), SIGNAL(progress(qint64, qint64)));
	legacyDownloadJob.reset(dljob);
	legacyDownloadJob->start();
}

void LegacyUpdate::fmllibsFinished()
{
	legacyDownloadJob.reset();
	if(!fmlLibsToProcess.isEmpty())
	{
		setStatus(tr("Copying FML libraries into the instance..."));
		LegacyInstance *inst = (LegacyInstance *)m_inst;
		auto metacache = MMC->metacache();
		int index = 0;
		for (auto &lib : fmlLibsToProcess)
		{
			progress(index, fmlLibsToProcess.size());
			auto entry = metacache->resolveEntry("fmllibs", lib.name);
			auto path = PathCombine(inst->libDir(), lib.name);
			if(!ensureFilePathExists(path))
			{
				emitFailed(tr("Failed creating FML library folder inside the instance."));
				return;
			}
			if (!QFile::copy(entry->getFullPath(), PathCombine(inst->libDir(), lib.name)))
			{
				emitFailed(tr("Failed copying Forge/FML library: %1.").arg(lib.name));
				return;
			}
			index++;
		}
		progress(index, fmlLibsToProcess.size());
	}
	lwjglStart();
}

void LegacyUpdate::fmllibsFailed()
{
	emitFailed("Game update failed: it was impossible to fetch the required FML libraries.");
	return;
}

void LegacyUpdate::lwjglStart()
{
	LegacyInstance *inst = (LegacyInstance *)m_inst;

	lwjglVersion = inst->lwjglVersion();
	lwjglTargetPath = PathCombine(MMC->settings()->get("LWJGLDir").toString(), lwjglVersion);
	lwjglNativesPath = PathCombine(lwjglTargetPath, "natives");

	// if the 'done' file exists, we don't have to download this again
	QFileInfo doneFile(PathCombine(lwjglTargetPath, "done"));
	if (doneFile.exists())
	{
		jarStart();
		return;
	}

	auto list = MMC->lwjgllist();
	if (!list->isLoaded())
	{
		emitFailed("Too soon! Let the LWJGL list load :)");
		return;
	}

	setStatus(tr("Downloading new LWJGL..."));
	auto version = list->getVersion(lwjglVersion);
	if (!version)
	{
		emitFailed("Game update failed: the selected LWJGL version is invalid.");
		return;
	}

	QString url = version->url();
	QUrl realUrl(url);
	QString hostname = realUrl.host();
	auto worker = MMC->qnam();
	QNetworkRequest req(realUrl);
	req.setRawHeader("Host", hostname.toLatin1());
	req.setHeader(QNetworkRequest::UserAgentHeader, "MultiMC/5.0 (Cached)");
	QNetworkReply *rep = worker->get(req);

	m_reply = std::shared_ptr<QNetworkReply>(rep);
	connect(rep, SIGNAL(downloadProgress(qint64, qint64)), SIGNAL(progress(qint64, qint64)));
	connect(worker.get(), SIGNAL(finished(QNetworkReply *)),
			SLOT(lwjglFinished(QNetworkReply *)));
	// connect(rep, SIGNAL(error(QNetworkReply::NetworkError)),
	// SLOT(downloadError(QNetworkReply::NetworkError)));
}

void LegacyUpdate::lwjglFinished(QNetworkReply *reply)
{
	if (m_reply.get() != reply)
	{
		return;
	}
	if (reply->error() != QNetworkReply::NoError)
	{
		emitFailed("Failed to download: " + reply->errorString() +
				   "\nSometimes you have to wait a bit if you download many LWJGL versions in "
				   "a row. YMMV");
		return;
	}
	auto worker = MMC->qnam();
	// Here i check if there is a cookie for me in the reply and extract it
	QList<QNetworkCookie> cookies =
		qvariant_cast<QList<QNetworkCookie>>(reply->header(QNetworkRequest::SetCookieHeader));
	if (cookies.count() != 0)
	{
		// you must tell which cookie goes with which url
		worker->cookieJar()->setCookiesFromUrl(cookies, QUrl("sourceforge.net"));
	}

	// here you can check for the 302 or whatever other header i need
	QVariant newLoc = reply->header(QNetworkRequest::LocationHeader);
	if (newLoc.isValid())
	{
		QString redirectedTo = reply->header(QNetworkRequest::LocationHeader).toString();
		QUrl realUrl(redirectedTo);
		QString hostname = realUrl.host();
		QNetworkRequest req(redirectedTo);
		req.setRawHeader("Host", hostname.toLatin1());
		req.setHeader(QNetworkRequest::UserAgentHeader, "MultiMC/5.0 (Cached)");
		QNetworkReply *rep = worker->get(req);
		connect(rep, SIGNAL(downloadProgress(qint64, qint64)),
				SIGNAL(progress(qint64, qint64)));
		m_reply = std::shared_ptr<QNetworkReply>(rep);
		return;
	}
	QFile saveMe("lwjgl.zip");
	saveMe.open(QIODevice::WriteOnly);
	saveMe.write(m_reply->readAll());
	saveMe.close();
	setStatus(tr("Installing new LWJGL..."));
	extractLwjgl();
	jarStart();
}
void LegacyUpdate::extractLwjgl()
{
	// make sure the directories are there

	bool success = ensureFolderPathExists(lwjglNativesPath);

	if (!success)
	{
		emitFailed("Failed to extract the lwjgl libs - error when creating required folders.");
		return;
	}

	QuaZip zip("lwjgl.zip");
	if (!zip.open(QuaZip::mdUnzip))
	{
		emitFailed("Failed to extract the lwjgl libs - not a valid archive.");
		return;
	}

	// and now we are going to access files inside it
	QuaZipFile file(&zip);
	const QString jarNames[] = {"jinput.jar", "lwjgl_util.jar", "lwjgl.jar"};
	for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
	{
		if (!file.open(QIODevice::ReadOnly))
		{
			zip.close();
			emitFailed("Failed to extract the lwjgl libs - error while reading archive.");
			return;
		}
		QuaZipFileInfo info;
		QString name = file.getActualFileName();
		if (name.endsWith('/'))
		{
			file.close();
			continue;
		}
		QString destFileName;
		// Look for the jars
		for (int i = 0; i < 3; i++)
		{
			if (name.endsWith(jarNames[i]))
			{
				destFileName = PathCombine(lwjglTargetPath, jarNames[i]);
			}
		}
		// Not found? look for the natives
		if (destFileName.isEmpty())
		{
#ifdef Q_OS_WIN32
			QString nativesDir = "windows";
#else
#ifdef Q_OS_MAC
			QString nativesDir = "macosx";
#else
			QString nativesDir = "linux";
#endif
#endif
			if (name.contains(nativesDir))
			{
				int lastSlash = name.lastIndexOf('/');
				int lastBackSlash = name.lastIndexOf('\\');
				if (lastSlash != -1)
					name = name.mid(lastSlash + 1);
				else if (lastBackSlash != -1)
					name = name.mid(lastBackSlash + 1);
				destFileName = PathCombine(lwjglNativesPath, name);
			}
		}
		// Now if destFileName is still empty, go to the next file.
		if (!destFileName.isEmpty())
		{
			setStatus(tr("Installing new LWJGL - extracting ") + name + "...");
			QFile output(destFileName);
			output.open(QIODevice::WriteOnly);
			output.write(file.readAll()); // FIXME: wste of memory!?
			output.close();
		}
		file.close(); // do not forget to close!
	}
	zip.close();
	m_reply.reset();
	QFile doneFile(PathCombine(lwjglTargetPath, "done"));
	doneFile.open(QIODevice::WriteOnly);
	doneFile.write("done.");
	doneFile.close();
}

void LegacyUpdate::lwjglFailed()
{
	emitFailed("Bad stuff happened while trying to get the lwjgl libs...");
}

void LegacyUpdate::jarStart()
{
	LegacyInstance *inst = (LegacyInstance *)m_inst;
	if (!inst->shouldUpdate() || inst->shouldUseCustomBaseJar())
	{
		ModTheJar();
		return;
	}

	setStatus(tr("Checking for jar updates..."));
	// Make directories
	QDir binDir(inst->binDir());
	if (!binDir.exists() && !binDir.mkpath("."))
	{
		emitFailed("Failed to create bin folder.");
		return;
	}

	// Build a list of URLs that will need to be downloaded.
	setStatus(tr("Downloading new minecraft.jar ..."));

	QString version_id = inst->intendedVersionId();
	QString localPath = version_id + "/" + version_id + ".jar";
	QString urlstr = "http://" + URLConstants::AWS_DOWNLOAD_VERSIONS + localPath;

	auto dljob = new NetJob("Minecraft.jar for version " + version_id);

	auto metacache = MMC->metacache();
	auto entry = metacache->resolveEntry("versions", localPath);
	dljob->addNetAction(CacheDownload::make(QUrl(urlstr), entry));
	connect(dljob, SIGNAL(succeeded()), SLOT(jarFinished()));
	connect(dljob, SIGNAL(failed()), SLOT(jarFailed()));
	connect(dljob, SIGNAL(progress(qint64, qint64)), SIGNAL(progress(qint64, qint64)));
	legacyDownloadJob.reset(dljob);
	legacyDownloadJob->start();
}

void LegacyUpdate::jarFinished()
{
	// process the jar
	ModTheJar();
}

void LegacyUpdate::jarFailed()
{
	// bad, bad
	emitFailed("Failed to download the minecraft jar. Try again later.");
}

bool LegacyUpdate::MergeZipFiles(QuaZip *into, QFileInfo from, QSet<QString> &contained,
								 MetainfAction metainf)
{
	setStatus(tr("Installing mods: Adding ") + from.fileName() + " ...");

	QuaZip modZip(from.filePath());
	modZip.open(QuaZip::mdUnzip);

	QuaZipFile fileInsideMod(&modZip);
	QuaZipFile zipOutFile(into);
	for (bool more = modZip.goToFirstFile(); more; more = modZip.goToNextFile())
	{
		QString filename = modZip.getCurrentFileName();
		if (filename.contains("META-INF") && metainf == LegacyUpdate::IgnoreMetainf)
		{
			QLOG_INFO() << "Skipping META-INF " << filename << " from " << from.fileName();
			continue;
		}
		if (contained.contains(filename))
		{
			QLOG_INFO() << "Skipping already contained file " << filename << " from "
						<< from.fileName();
			continue;
		}
		contained.insert(filename);
		QLOG_INFO() << "Adding file " << filename << " from " << from.fileName();

		if (!fileInsideMod.open(QIODevice::ReadOnly))
		{
			QLOG_ERROR() << "Failed to open " << filename << " from " << from.fileName();
			return false;
		}
		/*
		QuaZipFileInfo old_info;
		fileInsideMod.getFileInfo(&old_info);
		*/
		QuaZipNewInfo info_out(fileInsideMod.getActualFileName());
		/*
		info_out.externalAttr = old_info.externalAttr;
		*/
		if (!zipOutFile.open(QIODevice::WriteOnly, info_out))
		{
			QLOG_ERROR() << "Failed to open " << filename << " in the jar";
			fileInsideMod.close();
			return false;
		}
		if (!JlCompress::copyData(fileInsideMod, zipOutFile))
		{
			zipOutFile.close();
			fileInsideMod.close();
			QLOG_ERROR() << "Failed to copy data of " << filename << " into the jar";
			return false;
		}
		zipOutFile.close();
		fileInsideMod.close();
	}
	return true;
}

void LegacyUpdate::ModTheJar()
{
	LegacyInstance *inst = (LegacyInstance *)m_inst;

	if (!inst->shouldRebuild())
	{
		emitSucceeded();
		return;
	}

	// Get the mod list
	auto modList = inst->jarModList();

	QFileInfo runnableJar(inst->runnableJar());
	QFileInfo baseJar(inst->baseJar());
	bool base_is_custom = inst->shouldUseCustomBaseJar();

	// Nothing to do if there are no jar mods to install, no backup and just the mc jar
	if (base_is_custom)
	{
		// yes, this can happen if the instance only has the runnable jar and not the base jar
		// it *could* be assumed that such an instance is vanilla, but that wouldn't be safe
		// because that's not something mmc4 guarantees
		if (runnableJar.isFile() && !baseJar.exists() && modList->empty())
		{
			inst->setShouldRebuild(false);
			emitSucceeded();
			return;
		}

		setStatus(tr("Installing mods: Backing up minecraft.jar ..."));
		if (!baseJar.exists() && !QFile::copy(runnableJar.filePath(), baseJar.filePath()))
		{
			emitFailed("It seems both the active and base jar are gone. A fresh base jar will "
					   "be used on next run.");
			inst->setShouldRebuild(true);
			inst->setShouldUpdate(true);
			inst->setShouldUseCustomBaseJar(false);
			return;
		}
	}

	if (!baseJar.exists())
	{
		emitFailed("The base jar " + baseJar.filePath() + " does not exist");
		return;
	}

	if (runnableJar.exists() && !QFile::remove(runnableJar.filePath()))
	{
		emitFailed("Failed to delete old minecraft.jar");
		return;
	}

	// TaskStep(); // STEP 1
	setStatus(tr("Installing mods: Opening minecraft.jar ..."));

	QuaZip zipOut(runnableJar.filePath());
	if (!zipOut.open(QuaZip::mdCreate))
	{
		QFile::remove(runnableJar.filePath());
		emitFailed("Failed to open the minecraft.jar for modding");
		return;
	}
	// Files already added to the jar.
	// These files will be skipped.
	QSet<QString> addedFiles;

	// Modify the jar
	setStatus(tr("Installing mods: Adding mod files..."));
	for (int i = modList->size() - 1; i >= 0; i--)
	{
		auto &mod = modList->operator[](i);

		// do not merge disabled mods.
		if (!mod.enabled())
			continue;

		if (mod.type() == Mod::MOD_ZIPFILE)
		{
			if (!MergeZipFiles(&zipOut, mod.filename(), addedFiles, LegacyUpdate::KeepMetainf))
			{
				zipOut.close();
				QFile::remove(runnableJar.filePath());
				emitFailed("Failed to add " + mod.filename().fileName() + " to the jar.");
				return;
			}
		}
		else if (mod.type() == Mod::MOD_SINGLEFILE)
		{
			auto filename = mod.filename();
			if (!JlCompress::compressFile(&zipOut, filename.absoluteFilePath(),
										  filename.fileName()))
			{
				zipOut.close();
				QFile::remove(runnableJar.filePath());
				emitFailed("Failed to add " + filename.fileName() + " to the jar");
				return;
			}
			addedFiles.insert(filename.fileName());
			QLOG_INFO() << "Adding file " << filename.fileName() << " from "
						<< filename.absoluteFilePath();
		}
		else if (mod.type() == Mod::MOD_FOLDER)
		{
			auto filename = mod.filename();
			QString what_to_zip = filename.absoluteFilePath();
			QDir dir(what_to_zip);
			dir.cdUp();
			QString parent_dir = dir.absolutePath();
			if (!JlCompress::compressSubDir(&zipOut, what_to_zip, parent_dir, true, addedFiles))
			{
				zipOut.close();
				QFile::remove(runnableJar.filePath());
				emitFailed("Failed to add " + filename.fileName() + " to the jar");
				return;
			}
			QLOG_INFO() << "Adding folder " << filename.fileName() << " from "
						<< filename.absoluteFilePath();
		}
	}

	if (!MergeZipFiles(&zipOut, baseJar, addedFiles, LegacyUpdate::IgnoreMetainf))
	{
		zipOut.close();
		QFile::remove(runnableJar.filePath());
		emitFailed("Failed to insert minecraft.jar contents.");
		return;
	}

	// Recompress the jar
	zipOut.close();
	if (zipOut.getZipError() != 0)
	{
		QFile::remove(runnableJar.filePath());
		emitFailed("Failed to finalize minecraft.jar!");
		return;
	}
	inst->setShouldRebuild(false);
	// inst->UpdateVersion(true);
	emitSucceeded();
	return;
}
