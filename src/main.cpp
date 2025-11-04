#include <fstream>
#include "AxiomSecrets.h"
#include "NTFSLib/NTFS.hpp"

static CNTFSVolume NTFS_OpenDisk(wchar_t letter)
{
    CNTFSVolume volume(letter);

    if (!volume.IsVolumeOK())
    {
        printf("[!] Not a valid volume name\n");
        exit(1);
    }
    printf("[+] Volume opened\n");
    return (volume);
}

static CFileRecord NTFS_ParseRoot(CNTFSVolume *volume)
{
    CFileRecord fr(volume);
    fr.SetAttrMask(MASK_INDEX_ROOT | MASK_INDEX_ALLOCATION);
    if (!fr.ParseFileRecord(MFT_IDX_ROOT))
    {
        printf("Cannot read root directory of volume\n");
        exit(1);
    }
    printf("[+] Root directory parsed\n");

    if (!fr.ParseAttrs())
    {
        printf("[!] Cannot parse attributes\n");
        exit(1);
    }
    return (fr);
}

static int NTFS_TraverseDirectories(CFileRecord *fr, CIndexEntry *ie, const char **targets)
{
	size_t i;

	if (targets == NULL)
		return (0);

	i = 1;
	while (1)
	{
		if (targets[i] != NULL && targets[i + 1] == NULL)
			break;
		if (fr->FindSubEntry(targets[i], *ie))
		{
			printf("[+] Subdirectory %s found\n", targets[i]);
			if (!fr->ParseFileRecord(ie->GetFileReference()))
			{
				printf("[!] Cannot read root directory of volume\n");
				exit(1);
			}

			if (!fr->ParseAttrs())
			{
				if (fr->IsCompressed())
				    printf("[!] Compressed directory not supported yet\n");
				else if (fr->IsEncrypted())
				    printf("[!] Encrypted directory not supported yet\n");
				else
				    printf("[!] Cannot parse directory attributes\n");
				exit(1);
			}
		}
		else
		{
			printf("[!] Sub-directory %s not found\n", targets[i]);
			exit(1);
		}
		i++;
	}
	printf("[+] All subdirectories have been traversed\n");
	return (i);
}

static void NTFS_CopyFile(const char *filename, const char *savedir, CFileRecord *fr, CIndexEntry *ie)
{
	if (fr->FindSubEntry(filename, *ie))
	{
		if (!fr->ParseFileRecord(ie->GetFileReference()))
		{
			printf("[!] Cannot read file: %s\n", filename);
			return;
		}
		printf("[+] File %s opened\n", filename);

		fr->SetAttrMask(MASK_DATA);
		if (!fr->ParseAttrs())
		{
			if (fr->IsCompressed())
				printf("[!] Compressed file not supported yet\n");
			else if (fr->IsEncrypted())
				printf("[!] Encrypted file not supported yet\n");
			else
				printf("[!] Cannot parse file attributes\n");
			return;
		}
		printf("[+] Attributes set for %s\n", filename);

		const CAttrBase* data = fr->FindStream();
		if (data)
		{
			std::ofstream _file;
			DWORD datalen;
			DWORD readLen;
			DWORD totalReadLen;
			size_t readSize;
			BYTE filebuf[16 * 1024];
			char filepath[MAX_PATH + 1];

			readLen = 0;
			totalReadLen = 0;
			datalen = (DWORD)data->GetDataSize();
			printf("[+] %s size: %ld\n", filename, datalen);
			snprintf(filepath, sizeof(filepath), "%s\\%s.bak", savedir, filename);
			_file.open(filepath, std::ios::out | std::ios::binary | std::ios::trunc);
			while (totalReadLen < datalen)
			{
				readSize = datalen - totalReadLen;
				if (datalen - readLen > sizeof(filebuf))
					readSize = sizeof(filebuf);
				if (data->ReadData(totalReadLen, filebuf, readSize, &readLen))
				{
					_file.write((const char*)filebuf, readLen);
					totalReadLen += readLen;
				}
				else
				{
					printf("[!] Read data error for: %s\n", filename);
					_file.close();
					return;
				}
			}
			_file.close();
			printf("[+] File %s backed up\n", filename);
		}
	}
	else {
		printf("[!] File %s not found\n", filename);
	}
}

int main(int argc, char **argv)
{
	int argi;
	int filei;
	size_t i = 0;
	wchar_t volumeLetter;
	const char **targets;
	const char *savedir;

	if (argc < 2)
	{
		printf("[!] Usage: %s FILEPATH_1 [{FILEPATH_2}, {FILEPATH_3}, ...] SAVEDIR_PATH\n", argv[0]);
		return (1);
	}

	argi = 1;
	savedir = argv[argc - 1];
	while (argi < argc - 1)
	{

		targets = UTILS_strsplit(argv[argi], '\\');
		mbtowc(&volumeLetter, targets[0], 1);
		CIndexEntry ie;
		CNTFSVolume volume = NTFS_OpenDisk(volumeLetter);
		CFileRecord fr = NTFS_ParseRoot(&volume);
		filei = NTFS_TraverseDirectories(&fr, &ie, targets);
		NTFS_CopyFile(targets[filei], savedir, &fr, &ie);
		while (targets && targets[i] != NULL)
		{
			free((void*)targets[i]);
			i++;
		}
		free(targets);
		argi++;
	}
	return (0);
}

