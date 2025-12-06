Move-Item ./mod.template.json mod.template.main.json
Move-Item ./mod.template.tests.json ./mod.template.json

# generate zip to use as test lib qmod
qpm qmod zip --exclude_libs libbeatsaber-hook.so

# create mod.json with only regular bshook as dependency
if($LASTEXITCODE -eq 0) {
    qpm qmod manifest --exclude_libs libbeatsaber-hook.so

    $modJson = Get-Content ./mod.json -Raw | ConvertFrom-Json
    $id = $modJson.id
    $version = $modJson.version
    $modJson.id = "test-$id"
    $modJson.dependencies = @(@{"id" = "$id"; "version" = "^$version"})
    $modJson | ConvertTo-Json | Out-File ./mod.json

    # replace mod.json with modified version
    Compress-Archive -Path mod.json -Update -DestinationPath "$id.qmod"
    Move-Item "$id.qmod" "$id-tests.qmod" -Force
    Remove-Item mod.json
}

Move-Item ./mod.template.json mod.template.tests.json
Move-Item ./mod.template.main.json ./mod.template.json

# generate regular bshook zip
if($LASTEXITCODE -eq 0) {
    qpm qmod zip
}