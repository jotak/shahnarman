cost = {8,0,0,0}
icon = "spl_greatwyrm"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Grande guivre"
	else
		return "Great wyrm"
	end
end

function getDescription()
	desc = getUnitDescription("GreatWyrm")
	if language == "french" then
		return "Invoque une grande guivre\n"..desc
	else
		return "Summons a great wyrm\n"..desc
	end
end

function onCast()
	selectTarget("tile", "inrange")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	summon(caster, x, y, "GreatWyrm")
end
