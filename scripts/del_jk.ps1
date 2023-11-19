$targetPath = "./win"

# 搜索并删除 .pdb 文件
Get-ChildItem -Path $targetPath -Filter "*.pdb" -Recurse | foreach {
    if (-not $_.PSIsContainer) {
        Remove-Item -Path $_.FullName -Force
    }
}

# 搜索并删除 .lib 文件
Get-ChildItem -Path $targetPath -Filter "*.lib" -Recurse | foreach {
    if (-not $_.PSIsContainer) {
        Remove-Item -Path $_.FullName -Force
    }
}