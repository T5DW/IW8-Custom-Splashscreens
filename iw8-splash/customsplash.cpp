// IW8-Splas Screens
// this code is ment for people who want to make custom splash screens for there Iw8 project this method works across other games too like
// VG,MWIII, (Maybe) MWII 

#include "common.hpp"
#include "hooks.h"

utils::hook::detour LoadImageA_hk;

inline bool file_exists(const char* name)
{
    if (!name || *name == '\0')
    {
        return false;
    }

    struct stat buffer;
    return (stat(name, &buffer) == 0);
}

std::string GetPath()
{
    char buffer[MAX_PATH + 1]{};

    if (_getcwd(buffer, MAX_PATH) != nullptr)
    {
        return std::string(buffer);
    }

    printf("Error getting current working directory!");
    return "";
}

std::string GetCustomSplashPath(LPCSTR name)
{
    std::filesystem::path splashDir = std::filesystem::path(GetPath()) / ".youriw8project" / "splash";

    std::error_code ec;
    std::filesystem::create_directories(splashDir, ec);

    if (ec)
    {
        printf("Failed to create splash directory: %s\n", ec.message().c_str());
    }

    if (name && name[0] != '\0')
    {
        return (splashDir / std::filesystem::path(name).filename()).string();
    }

    return (splashDir / "splash.bmp").string();
}

std::string GetBmpNamesMessage()
{
    const std::filesystem::path splashDir = std::filesystem::path(GetPath()) / ".youriw8project" / "splash";

    std::error_code ec;
    std::filesystem::create_directories(splashDir, ec);

    std::string message = "No BMP files found in .youriw8project/splash";

    std::vector<std::string> bmpNames;
    if (std::filesystem::exists(splashDir, ec) && std::filesystem::is_directory(splashDir, ec))
    {
        for (const auto& entry : std::filesystem::directory_iterator(splashDir, ec))
        {
            if (entry.is_regular_file(ec) && entry.path().extension() == ".bmp")
            {
                bmpNames.push_back(entry.path().filename().string());
            }
        }
    }

    if (!bmpNames.empty())
    {
        message.clear();
        for (size_t i = 0; i < bmpNames.size(); ++i)
        {
            if (i > 0)
            {
                message += "\n";
            }

            message += bmpNames[i];
        }
    }

    return message;
}

void ShowSplashBmpPopup()
{
    const std::string bmpNames = GetBmpNamesMessage();
    MessageBoxA(nullptr, bmpNames.c_str(), "youriw8project", MB_OK | MB_ICONINFORMATION);
}

HBITMAP WINAPI LoadImageA_Detour(
    HINSTANCE hInst,
    LPCSTR name,
    UINT type,
    int cx,
    int cy,
    UINT fuLoad)
{
    const std::string customPath = GetCustomSplashPath(name);

    if (file_exists(customPath.c_str()))
    {
        return LoadImageA_hk.stub<HBITMAP>(
            nullptr,
            customPath.c_str(),
            IMAGE_BITMAP,
            cx,
            cy,
            fuLoad | LR_LOADFROMFILE
        );
    }

    return LoadImageA_hk.stub<HBITMAP>(
        hInst,
        name,
        type,
        cx,
        cy,
        fuLoad
    );
}

void AutoLoad()
{
    static bool popupShown = false;
    if (!popupShown)
    {
        popupShown = true;
        ShowSplashBmpPopup();
    }

    LoadImageA_hk.create(LoadImageA, LoadImageA_Detour);
}
