cost = {2,0,0,0}
icon = "spl_killingsprites"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Fées tueuses"
	else
		return "Killing sprites"
	end
end

function getDescription()
	desc = getUnitDescription("KillingSprites")
	if language == "french" then
		return "Invoque des fées tueuses\n"..desc
	else
		return "Summons killing sprites\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "KillingSprites")
end
