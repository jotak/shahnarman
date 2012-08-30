cost = {0,0,0,2}
icon = "spellstealing"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Vol de sort actif"
	else
		return "Active spell stealing"
	end
end

function getDescription()
	if language == "french" then
		return "Choisissez un sort actif, et prenez-le en main."
	else
		return "Choose an active spell, and take it in hand."
	end
end

function onCast()
	selectTarget("spell_in_play", "")
end

function onResolve(params, caster)
	player, spell = split(params, " ")
	changeSpellOwner("spell_in_play", player, spell, caster)
	recallSpell("spell_in_play", caster, spell)
end
