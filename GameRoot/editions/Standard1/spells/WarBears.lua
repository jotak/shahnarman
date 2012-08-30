cost = {3,0,0,0}
icon = "spl_warbears"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Ours de guerre"
	else
		return "War bears"
	end
end

function getDescription()
	desc = getUnitDescription("WarBears")
	if language == "french" then
		return "Invoque des ours de guerre\n"..desc
	else
		return "Summons war bears\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "WarBears")
end
