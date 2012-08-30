cost = {0,0,0,6}
icon = "spl_ragingspirit"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Esprit enragé"
	else
		return "Raging spirit"
	end
end

function getDescription()
	desc = getUnitDescription("RagingSpirit")
	if language == "french" then
		return "Invoque un esprit enragé\n"..desc
	else
		return "Summons a raging spirit\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "RagingSpirit")
end
