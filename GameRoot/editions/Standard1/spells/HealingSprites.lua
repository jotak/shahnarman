cost = {2,0,0,0}
icon = "spl_healingsprites"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "F�es gu�risseuses"
	else
		return "Healing sprites"
	end
end

function getDescription()
	desc = getUnitDescription("HealingSprites")
	if language == "french" then
		return "Invoque des f�es gu�risseuses\n"..desc
	else
		return "Summons healing sprites\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "HealingSprites")
end
