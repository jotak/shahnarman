cost = {0,3,0,0}
icon = "spl_crusader"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Croisé"
	else
		return "Crusader"
	end
end

function getDescription()
	desc = getUnitDescription("Crusader")
	if language == "french" then
		return "Invoque un croisé\n"..desc
	else
		return "Summons a crusader\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Crusader")
end
