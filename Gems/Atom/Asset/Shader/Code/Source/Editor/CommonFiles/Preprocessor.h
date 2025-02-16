/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <AzCore/std/string/string.h>
#include <AzCore/std/containers/set.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/RTTI/ReflectContext.h>
#include <AzCore/Serialization/DataPatch.h>

namespace AZ
{
    namespace ShaderBuilder
    {
        //! Collects data output from the PreprocessFile() function
        struct PreprocessorData
        {
            //! Will contain the preprocessed code.
            AZStd::string code;

            //! May contain warning and error messages, if this option is enabled in PreprocessFile().
            AZStd::string diagnostics;
            
            //! Will contain the entire inclusion tree, not just the files include by top level AZSL file.
            AZStd::set<AZStd::string> includedPaths;
        };

        //! Object to store preprocessor options, as will be passed in the command line.
        struct PreprocessorOptions final
        {
            AZ_RTTI(PreprocessorOptions, "{684181FC-7372-49FC-B69C-1FF510A29621}");
            AZ_CLASS_ALLOCATOR(PreprocessorOptions, AZ::SystemAllocator, 0);

            static void Reflect(AZ::ReflectContext* context);

            //! passed as -I folder1 -I folder2...
            //! folders are relative to the dev folder of the project
            AZStd::vector<AZStd::string> m_projectIncludePaths;

            //! Each string is of the type "name[=value]"
            //! passed as -Dmacro1[=value1] -Dmacro2 ... to MCPP.
            AZStd::vector<AZStd::string> m_predefinedMacros;

            //! Removes all macros from @m_predefinedMacros that appear in @macroNames
            void RemovePredefinedMacros(const AZStd::vector<AZStd::string>& macroNames);

            //! if needed, we may add configurations like
            //!   "keep comments" or "don't predefine non-standard macros"
            //!   or "output diagnostics to std.err" or "enable digraphs/trigraphs"...
        };

        //! You can use this canonicalized way to initialize preprocessor options
        //! It will populate your option with a default base of include folders given by the Asset Processor scan folders.
        //! This is going to look for a Config/shader_global_build_options.json in one of the scan folders
        //!  (that file can specify additional include files and preprocessor macros).
        //! @param options: Outout parameter, will contain the preprocessor options.
        //! @param builderName: Used for debugging.
        //! @param optionalIncludeFolder: If not null, will be added to the list of include folders for the c-preprocessor in @options.
        void InitializePreprocessorOptions(PreprocessorOptions& options, const char* builderName, const char* optionalIncludeFolder = nullptr);

        /**
        * Runs the preprocessor on the given source file path, and stores results in outputData.
        * @param fullPath   The file to preprocess
        * @param outputData Collects data from the preprocessor. This will be filled out as much as possible, even if preprocessing fails.
        * @param options    Control of macros to define and paths to solve includes
        * @param collectDiagnostics If true, warnings and errors will be collected in outputData.diagnostics instead of using AZ_Warning and AZ_Error.
        * @param preprocessIncludedFiles  By default MCPP follows the chain of included files
        *        and extracts the content of each file and dumps it in the output.
        *        Setting this flag to false will prevent mcpp from preprocessing the included files,
        *        so the produced content will come only from the file given as input to MCPP.
        *        Setting to false is handy, for example, for the SrgLayoutBuilder from creating SRGs from included files.
        *        REMARK: You can make the argument of why not simply leave the @m_projectIncludePaths empty?
        *        It will cause MCPP to error because it won't find the included files. So, in reality the chain
        *        of included files is validated, but their content won't make it into the output.
        *        A change is required in azslc so it skips #include lines. SEE: [ATOM-5302]
        * @return false if the preprocessor failed
        */
        bool PreprocessFile(
            const AZStd::string& fullPath,
            PreprocessorData& outputData,
            const PreprocessorOptions& options,
            bool collectDiagnostics = false,
            bool preprocessIncludedFiles = true);

        //! Replace all ocurrences of #line "whatever" by #line "whatwewant"
        void MutateLineDirectivesFileOrigin(
            AZStd::string& sourceCode,
            AZStd::string newFileOrigin);

    } // ShaderBuilder
} // AZ
