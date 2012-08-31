cost = {0,4,0,0}
icon = "spl_justicechampion"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Champion justicier"
	else
		return "Justice champion"
	end
end

function getDescription()
	desc = getUnitDescription("JusticeChampion")
	if language == "french" then
		return "Invoque un champion justicier n'importe où sur la carte\n"..desc
	else
		return "Summons anywhere a justice champion\n"..desc
	end
end

function onCast()
	selectTarget("tile", "")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "JusticeChampion")
end
