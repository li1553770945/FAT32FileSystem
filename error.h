#pragma once

enum {
	ERR_None = 0,
	ERR_Unknown,
	ERR_Todo,
	ERR_KmallocFailed,
	ERR_AccessInvalidVMR,
	ERR_AccessPermissionViolate,
	ERR_BusyForking,
	ERR_InvalidParameter,//for unknown specific reasons
	ERR_FileOperationOutofRange,
	ERR_VertifyNumberDisagree,
	ERR_VersionError,
	ERR_FileAlreadyExist,
	ERR_FilePathNotExist,
	ERR_DirectoryPathNotExist,
	ERR_PathIsNotDirectory,
	ERR_PathIsNotFile,
	ERR_FileNameTooLong,
	ERR_SystemError,
};