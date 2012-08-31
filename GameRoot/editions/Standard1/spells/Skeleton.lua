cost = {0,0,2,0}
icon = "spl_skeleton"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Squelettes"
	else
		return "Skeletons"
	end
end

function getDescription()
	desc = getUnitDescription("Skeleton")
	if language == "french" then
		return "Invoque des squelettes\n"..desc
	else
		return "Summons skeletons\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "Skeleton")
end
