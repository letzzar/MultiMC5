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

#pragma once

#include "BaseInstaller.h"

#include <QString>
#include <memory>

class VersionFinal;

class ForgeInstaller : public BaseInstaller
{
public:
	ForgeInstaller();

	void prepare(const QString &filename, const QString &universalUrl);
	bool add(OneSixInstance *to) override;

	QString id() const override { return "net.minecraftforge"; }

	ProgressProvider *createInstallTask(OneSixInstance *instance, BaseVersionPtr version, QObject *parent) override;

private:
	// the version, read from the installer
	std::shared_ptr<VersionFinal> m_forge_version;
	QString internalPath;
	QString finalPath;
	QString realVersionId;
	QString m_forgeVersionString;
	QString m_universal_url;
};
