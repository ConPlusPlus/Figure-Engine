Launcher updater notes
======================

The launcher reads update metadata from:
  updates.ini
next to the launcher executable by default.

You can also set a remote URL in launcher.ini:
  [Updater]
  FeedUrl=https://your-domain.example.com/updates.ini

Feed format example:
  [LevelEditor]
  Version=1.0.1
  Title=Brush tool stability update
  Notes=Improved block save reliability
  DownloadUrl=https://your-domain.example.com/FigureLevelEditor.exe

How updates work:
- Configure each editor executable path on the Settings page.
- Open Updates and click Refresh.
- Click Download for the editor you want to update.
- The launcher downloads the replacement exe and overwrites the configured path.
- The launcher stores the installed version in the current .figproj.

Note:
- Editors should be closed before replacing their exe.
- Self-updating the launcher exe is not implemented in this pass.
